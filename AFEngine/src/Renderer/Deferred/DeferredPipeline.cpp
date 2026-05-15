#include "Renderer/Deferred/DeferredPipeline.h"

#include "Core/Engine.h"
#include "Core/Log.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/World.h"
#include "Renderer/Mesh.h"

namespace AF {

void DeferredRenderPipeline::OnSetup(Engine& engine)
{
    AF_LOG_INFO("DeferredRenderPipeline: setup...");

    using CS = RHI::ShaderStage;

    m_GBufferShader = RHI::RHIShader::Create("gbuffer", {
        { CS::Vertex,   "assets/shaders/gbuffer.vert" },
        { CS::Fragment, "assets/shaders/gbuffer.frag" },
    });
    m_LightingShader = RHI::RHIShader::Create("deferred_lighting", {
        { CS::Vertex,   "assets/shaders/deferred_lighting.vert" },
        { CS::Fragment, "assets/shaders/deferred_lighting.frag" },
    });
    m_CompositeShader = RHI::RHIShader::Create("composite", {
        { CS::Vertex,   "assets/shaders/composite.vert" },
        { CS::Fragment, "assets/shaders/composite.frag" },
    });
    m_FullscreenQuad = Mesh::CreateQuad(2.0f);

    m_CameraUBO = RHI::RHIUniformBuffer::Create(sizeof(CameraGPU), 0);

    auto& world = engine.GetWorld();
    auto mat = std::make_shared<Material>(m_GBufferShader);
    auto meshView = world.View<TransformComponent, MeshComponent>();
    for (auto [enttHandle, transform, mesh] : meshView.each())
    {
        Entity entity(enttHandle, &world);
        if (!entity.HasComponent<MaterialComponent>())
            entity.AddComponent<MaterialComponent>(mat);
    }
    AF_LOG_INFO("DeferredRenderPipeline: added materials to {} entities", meshView.size_hint());
}

void DeferredRenderPipeline::OnRender(const RenderView& view, const RenderPacket& packet,
                                      RHI::RHICommandBuffer& cmdBuf)
{
    CameraGPU cam;
    cam.ViewProjection        = view.ViewProjection;
    cam.InverseViewProjection = glm::inverse(view.ViewProjection);
    cam.Projection            = view.Projection;
    cam.Position              = view.Position;
    cam.ScreenSize            = { static_cast<float>(view.Width), static_cast<float>(view.Height) };

    cmdBuf.SetBufferData(m_CameraUBO.get(), &cam, sizeof(CameraGPU));
    cmdBuf.BindUniformBuffer(m_CameraUBO.get(), 0);

    bool hasLight = !packet.Lights.empty();
    if (hasLight != m_HadLightLastFrame)
    {
        m_Graph.ClearNodes();
        m_HadLightLastFrame = hasLight;
    }

    uint32_t w = view.Width;
    uint32_t h = view.Height;

    auto gAlbedo    = m_Graph.CreateTexture("gAlbedo");
    auto gNormal    = m_Graph.CreateTexture("gNormal");
    auto gMaterial  = m_Graph.CreateTexture("gMaterial");
    auto gDepth     = m_Graph.CreateTexture("gDepth");
    auto lightAccum    = m_Graph.CreateTexture("lightAccum");
    auto finalComposite = m_Graph.CreateTexture("finalComposite");

    SetupGBufferPass(w, h, gAlbedo, gNormal, gMaterial, gDepth);

    if (hasLight)
    {
        SetupLightingPass(w, h, gAlbedo, gNormal, gMaterial, gDepth, lightAccum);
        SetupCompositePass(w, h, lightAccum, finalComposite);
    }
}

void DeferredRenderPipeline::SetupGBufferPass(uint32_t width, uint32_t height,
    RenderResource gAlbedo, RenderResource gNormal,
    RenderResource gMaterialRes, RenderResource gDepth)
{
    RenderPassDesc desc;
    desc.Width  = width;
    desc.Height = height;
    desc.Attachments = {
        RHI::TextureFormat::RGBA8,
        RHI::TextureFormat::RGBA16F,
        RHI::TextureFormat::RGBA8,
        RHI::TextureFormat::Depth32
    };
    desc.Clear = true;

    auto& node = m_Graph.AddNode("GBuffer", desc);
    node.Write(gAlbedo);
    node.Write(gNormal);
    node.Write(gMaterialRes);
    node.Write(gDepth);

    node.SetExecute([this](RenderGraphContext& ctx) {
        auto& cmd = *ctx.Cmd;
        auto& packet = ctx.GetPacket();

        cmd.SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        cmd.Clear();

        for (auto& snap : packet.Entities)
        {
            if (!snap.MeshSource) continue;

            if (snap.MaterialSource)
                snap.MaterialSource->RecordBind(cmd);
            else
                cmd.BindShader(m_GBufferShader);

            cmd.SetMat4("u_Model", snap.Transform);
            cmd.DrawIndexed(snap.MeshSource->GetVertexArray(),
                            snap.MeshSource->GetIndexCount());
        }
    });
}

void DeferredRenderPipeline::SetupLightingPass(uint32_t width, uint32_t height,
    RenderResource gAlbedo, RenderResource gNormal,
    RenderResource gMaterialRes, RenderResource gDepth, RenderResource lightAccum)
{
    RenderPassDesc desc;
    desc.Width  = width;
    desc.Height = height;
    desc.Attachments = { RHI::TextureFormat::RGBA16F };
    desc.Clear = true;

    auto& node = m_Graph.AddNode("Lighting", desc);
    node.Read(gAlbedo);
    node.Read(gNormal);
    node.Read(gMaterialRes);
    node.Read(gDepth);
    node.Write(lightAccum);

    node.SetExecute([this](RenderGraphContext& ctx) {
        auto& cmd = *ctx.Cmd;
        auto& packet = ctx.GetPacket();

        cmd.SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        cmd.Clear();

        cmd.BindShader(m_LightingShader);

        cmd.BindTexture(0, ctx.GetInput("gAlbedo"));
        cmd.BindTexture(1, ctx.GetInput("gNormal"));
        cmd.BindTexture(2, ctx.GetInput("gMaterial"));
        cmd.BindTexture(3, ctx.GetInput("gDepth"));
        cmd.SetInt("u_Albedo", 0);
        cmd.SetInt("u_Normal", 1);
        cmd.SetInt("u_Material", 2);
        cmd.SetInt("u_Depth", 3);

        if (!packet.Lights.empty())
        {
            auto& light = packet.Lights[0];
            cmd.SetFloat3("u_LightDir", light.Direction);
            cmd.SetFloat3("u_LightColor", light.Color);
            cmd.SetFloat("u_LightIntensity", light.Intensity);
        }
        else
        {
            cmd.SetFloat3("u_LightDir", glm::vec3(0.0f, -1.0f, -1.0f));
            cmd.SetFloat3("u_LightColor", glm::vec3(1.0f));
            cmd.SetFloat("u_LightIntensity", 1.0f);
        }

        cmd.DrawIndexed(m_FullscreenQuad->GetVertexArray(),
                        m_FullscreenQuad->GetIndexCount());
    });
}

void DeferredRenderPipeline::SetupCompositePass(uint32_t width, uint32_t height,
    RenderResource lightAccum, RenderResource outputTarget)
{
    RenderPassDesc desc;
    desc.Width  = width;
    desc.Height = height;
    desc.Attachments = { RHI::TextureFormat::RGBA8 };
    desc.Clear = true;

    auto& node = m_Graph.AddNode("Composite", desc);
    node.Read(lightAccum);
    node.Write(outputTarget);

    node.SetExecute([this](RenderGraphContext& ctx) {
        auto& cmd = *ctx.Cmd;

        cmd.SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        cmd.Clear();

        cmd.BindShader(m_CompositeShader);
        cmd.BindTexture(0, ctx.GetInput("lightAccum"));
        cmd.SetInt("u_Input", 0);
        cmd.DrawIndexed(m_FullscreenQuad->GetVertexArray(),
                        m_FullscreenQuad->GetIndexCount());
    });
}

} // namespace AF
