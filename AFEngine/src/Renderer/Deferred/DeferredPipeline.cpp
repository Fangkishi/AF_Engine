#include "Renderer/Deferred/DeferredPipeline.h"

#include "Core/Engine.h"
#include "Core/Log.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/World.h"
#include "Renderer/Mesh.h"
#include "Factory/MeshFactory.h"
#include "Factory/MaterialFactory.h"

namespace AF {

void DeferredRenderPipeline::OnSetup(Engine& engine)
{
    AF_LOG_INFO("DeferredRenderPipeline: setup...");

    using CS = RHI::ShaderStage;

    m_GBufferShader = RHI::RHIShader::Create("gbuffer", {
        { CS::Vertex,   "assets/shaders/gbuffer.vert" },
        { CS::Fragment, "assets/shaders/gbuffer.frag" },
    });
    m_CompositeShader = RHI::RHIShader::Create("composite", {
        { CS::Vertex,   "assets/shaders/composite.vert" },
        { CS::Fragment, "assets/shaders/composite.frag" },
    });
    m_FullscreenQuad = MeshFactory::CreateQuad(2.0f);

    m_CameraUBO = RHI::RHIUniformBuffer::Create(sizeof(CameraGPU), 0);

    auto& world = engine.GetWorld();
    auto mat = MaterialFactory::CreateError();
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

    uint32_t w = view.Width;
    uint32_t h = view.Height;

    auto gAlbedo       = m_Graph.CreateTexture("gAlbedo");
    auto gNormal       = m_Graph.CreateTexture("gNormal");
    auto gMaterial     = m_Graph.CreateTexture("gMaterial");
    auto gDepth        = m_Graph.CreateTexture("gDepth");
    auto finalComposite = m_Graph.CreateTexture("finalComposite");

    SetupGBufferPass(w, h, gAlbedo, gNormal, gMaterial, gDepth);
    SetupCompositePass(w, h, gAlbedo, finalComposite);
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
    desc.Shader = m_GBufferShader;
    desc.DepthStencil.DepthTest  = true;
    desc.DepthStencil.DepthWrite = true;
    desc.DepthStencil.DepthFunc  = RHI::DepthCompareFunc::Less;
    desc.Rasterizer.Cull  = RHI::CullMode::Back;
    desc.Rasterizer.Winding = RHI::FrontFace::CCW;
    desc.Rasterizer.Fill  = RHI::FillMode::Solid;
    desc.InputLayout = {
        { RHI::ShaderDataType::Float3, "a_Position" },
        { RHI::ShaderDataType::Float3, "a_Normal"   },
        { RHI::ShaderDataType::Float4, "a_Tangent"  },
        { RHI::ShaderDataType::Float2, "a_TexCoord" },
        { RHI::ShaderDataType::Float4, "a_Color"    },
    };

    auto& node = m_Graph.AddNode("GBuffer", desc);
    node.Write(gAlbedo);
    node.Write(gNormal);
    node.Write(gMaterialRes);
    node.Write(gDepth);

    node.SetExecute([this](RenderGraphContext& ctx) {
        auto& cmd = *ctx.Cmd;
        auto& packet = ctx.GetPacket();

        cmd.SetClearColor({ 0.08f, 0.08f, 0.1f, 1.0f });
        cmd.Clear();

        for (auto& snap : packet.Entities)
        {
            if (!snap.MeshSource) continue;

            if (snap.MaterialSource)
                snap.MaterialSource->RecordBind(cmd);

            cmd.SetMat4("u_Model", snap.Transform);
            cmd.DrawIndexed(snap.MeshSource->GetVertexArray(),
                            snap.MeshSource->GetIndexCount());
        }
    });
}

void DeferredRenderPipeline::SetupCompositePass(uint32_t width, uint32_t height,
    RenderResource sourceTexture, RenderResource outputTarget)
{
    RenderPassDesc desc;
    desc.Width  = width;
    desc.Height = height;
    desc.Attachments = { RHI::TextureFormat::RGBA8 };
    desc.Clear = true;
    desc.Shader = m_CompositeShader;
    desc.DepthStencil.DepthTest  = false;
    desc.DepthStencil.DepthWrite = false;
    desc.Rasterizer.Cull  = RHI::CullMode::None;
    desc.Rasterizer.Winding = RHI::FrontFace::CCW;
    desc.Rasterizer.Fill  = RHI::FillMode::Solid;
    desc.InputLayout = {
        { RHI::ShaderDataType::Float3, "a_Position" },
        { RHI::ShaderDataType::Float2, "a_TexCoord" },
    };

    auto& node = m_Graph.AddNode("Composite", desc);
    node.Read(sourceTexture);
    node.Write(outputTarget);

    node.SetExecute([this](RenderGraphContext& ctx) {
        auto& cmd = *ctx.Cmd;

        cmd.SetClearColor({ 0.08f, 0.08f, 0.1f, 1.0f });
        cmd.Clear();

        cmd.BindTexture(0, ctx.GetInput("gAlbedo"));
        cmd.SetInt("u_Input", 0);
        cmd.DrawIndexed(m_FullscreenQuad->GetVertexArray(),
                        m_FullscreenQuad->GetIndexCount());
    });
}

} // namespace AF
