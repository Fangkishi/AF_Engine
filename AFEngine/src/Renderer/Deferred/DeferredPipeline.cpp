#include "Renderer/Deferred/DeferredPipeline.h"

#include "Core/Engine.h"
#include "Core/Log.h"
#include "Renderer/Mesh.h"
#include "Factory/MeshFactory.h"
#include "RHI/ShaderReflection.h"
#include "RHI/PipelineState.h"
#include "RenderGraph/PSOCache.h"
#include "ShaderVariant/ShaderLibrary.h"
#include "MaterialGraph/MaterialCompiler.h"

namespace AF {

void DeferredRenderPipeline::OnSetup(Engine& engine)
{
    AF_LOG_INFO("DeferredRenderPipeline: setup...");

    m_FullscreenQuad = MeshFactory::CreateQuad(2.0f);
    m_CameraUBO = RHI::RHIUniformBuffer::Create(sizeof(CameraGPU), 0);
    m_MaterialUBO = RHI::RHIUniformBuffer::Create(256, 1);
}

void DeferredRenderPipeline::OnRender(const RenderView& view, const RenderPacket& packet,
                                      RHI::RHICommandBuffer& cmdBuf)
{
    // 填充相机 UBO
    CameraGPU cam;
    cam.ViewProjection        = view.ViewProjection;
    cam.InverseViewProjection = glm::inverse(view.ViewProjection);
    cam.Projection            = view.Projection;
    cam.Position              = view.Position;
    cam.ScreenSize            = { static_cast<float>(view.Width), static_cast<float>(view.Height) };
    cam.ViewDirection         = view.Forward;
    cam.Time                  = GetEngine()->GetElapsedTime();
    cam.DeltaTime             = GetEngine()->GetDeltaTime();

    cmdBuf.SetBufferData(m_CameraUBO.get(), &cam, sizeof(CameraGPU));
    cmdBuf.BindUniformBuffer(m_CameraUBO.get(), 0);

    uint32_t w = view.Width;
    uint32_t h = view.Height;

    SetupGBufferPass(w, h);
    SetupCompositePass(w, h);
}

void DeferredRenderPipeline::SetupGBufferPass(uint32_t width, uint32_t height)
{
    RenderPassDesc desc;
    desc.Width  = width;
    desc.Height = height;
    desc.Attachments = { RHI::TextureFormat::RGBA8, RHI::TextureFormat::RGBA16F, RHI::TextureFormat::RGBA8, RHI::TextureFormat::Depth32 };
    desc.Clear = true;
    desc.ClearColor = { 0.08f, 0.08f, 0.1f, 1.0f };

    RHI::DepthStencilState depthStencil;
    depthStencil.DepthTest  = true;
    depthStencil.DepthWrite = true;
    depthStencil.DepthFunc  = RHI::DepthCompareFunc::Less;

    RHI::RasterizerState rasterizer;
    rasterizer.Cull    = RHI::CullMode::Back;
    rasterizer.Winding = RHI::FrontFace::CCW;
    rasterizer.Fill    = RHI::FillMode::Solid;

    RHI::BufferLayout gbufferLayout = {
        { RHI::ShaderDataType::Float3, "a_Position" },
        { RHI::ShaderDataType::Float3, "a_Normal"   },
        { RHI::ShaderDataType::Float4, "a_Tangent"  },
        { RHI::ShaderDataType::Float2, "a_TexCoord" },
        { RHI::ShaderDataType::Float4, "a_Color"    },
    };

    auto& node = m_Graph.AddNode("GBuffer", desc);
    node.WriteTo("gAlbedo");
    node.WriteTo("gNormal");
    node.WriteTo("gMaterial");
    node.WriteTo("gDepth");

    node.SetExecute([this, depthStencil, rasterizer, gbufferLayout](RenderGraphContext& ctx) {
        auto& cmd = *ctx.Cmd;
        auto& packet = ctx.GetPacket();
        auto& lib = ShaderLibrary::Get();
        auto& psoCache = PSOCache::Get();

        for (auto& snap : packet.Opaque)
        {
            if (!snap.MeshSource) continue;

            if (snap.MaterialSource)
                snap.MaterialSource->SetMaterialUBO(m_MaterialUBO.get());

            const ShaderSnippet* snippet = nullptr;
            static ShaderSnippet emptySnippet;
            if (snap.MaterialSource && snap.MaterialSource->Parent)
                snippet = &snap.MaterialSource->Parent->CompiledSnippet;
            if (!snippet || snippet->GLSLCode.empty())
                snippet = &emptySnippet;

            auto shader = lib.GetOrCreatePipelineVariant("gbuffer", *snippet);
            if (!shader) continue;

            PSODesc pso;
            pso.DepthStencil   = depthStencil;
            pso.Rasterizer     = rasterizer;
            pso.FragmentShader = shader;
            pso.InputLayout    = gbufferLayout;
            pso.ColorFormats   = { RHI::TextureFormat::RGBA8, RHI::TextureFormat::RGBA16F, RHI::TextureFormat::RGBA8 };
            pso.DepthFormat    = RHI::TextureFormat::Depth32;

            size_t hash = psoCache.GetOrCreate(pso);
            psoCache.Apply(hash, cmd);

            const auto* refl = lib.GetReflection("gbuffer", *snippet);
            if (snap.MaterialSource)
                snap.MaterialSource->RecordBind(cmd, refl ? *refl : ShaderReflection{});

            cmd.SetMat4("u_Model", snap.Transform);
            cmd.DrawIndexed(snap.MeshSource->GetVertexArray(), snap.MeshSource->GetIndexCount());
        }
    });
}

void DeferredRenderPipeline::SetupCompositePass(uint32_t width, uint32_t height)
{
    RenderPassDesc desc;
    desc.Width  = width;
    desc.Height = height;
    desc.Attachments = { RHI::TextureFormat::RGBA8 };
    desc.Clear = true;
    desc.ClearColor = { 0.08f, 0.08f, 0.1f, 1.0f };

    RHI::DepthStencilState depthStencil;
    depthStencil.DepthTest  = false;
    depthStencil.DepthWrite = false;

    RHI::RasterizerState rasterizer;
    rasterizer.Cull    = RHI::CullMode::None;
    rasterizer.Winding = RHI::FrontFace::CCW;
    rasterizer.Fill    = RHI::FillMode::Solid;

    RHI::BufferLayout compositeLayout = {
        { RHI::ShaderDataType::Float3, "a_Position" },
        { RHI::ShaderDataType::Float2, "a_TexCoord" },
    };

    auto& node = m_Graph.AddNode("Composite", desc);
    node.ReadFrom("gAlbedo");
    node.WriteTo("finalComposite");

    node.SetExecute([this, depthStencil, rasterizer, compositeLayout](RenderGraphContext& ctx) {
        auto& cmd = *ctx.Cmd;

        static ShaderSnippet emptySnippet;
        auto& lib = ShaderLibrary::Get();
        auto& psoCache = PSOCache::Get();

        auto shader = lib.GetOrCreatePipelineVariant("composite", emptySnippet, ctx.GetPassTextureSlots());
        if (!shader) return;

        PSODesc pso;
        pso.DepthStencil   = depthStencil;
        pso.Rasterizer     = rasterizer;
        pso.FragmentShader = shader;
        pso.InputLayout    = compositeLayout;
        pso.ColorFormats   = { RHI::TextureFormat::RGBA8 };

        size_t hash = psoCache.GetOrCreate(pso);
        psoCache.Apply(hash, cmd);

        cmd.DrawIndexed(m_FullscreenQuad->GetVertexArray(), m_FullscreenQuad->GetIndexCount());
    });
}

} // namespace AF
