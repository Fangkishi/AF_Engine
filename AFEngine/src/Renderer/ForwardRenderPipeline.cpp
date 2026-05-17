#include "Renderer/ForwardRenderPipeline.h"

#include "Core/Engine.h"
#include "Core/Log.h"
#include "Renderer/Renderer.h"
#include "Renderer/CameraGPU.h"
#include "Renderer/Mesh.h"
#include "Factory/MeshFactory.h"
#include "RHI/ShaderReflection.h"
#include "RHI/PipelineState.h"
#include "RenderGraph/PSOCache.h"
#include "ShaderVariant/ShaderLibrary.h"
#include "MaterialGraph/MaterialCompiler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace AF {

void ForwardRenderPipeline::OnSetup(Engine& engine)
{
    AF_LOG_INFO("ForwardRenderPipeline: setup...");
    m_FullscreenQuad = MeshFactory::CreateQuad(2.0f);
    m_CameraUBO = RHI::RHIUniformBuffer::Create(sizeof(CameraGPU), 0);
    m_MaterialUBO = RHI::RHIUniformBuffer::Create(256, 1);
}

void ForwardRenderPipeline::OnRender(const RenderView& view, const RenderPacket& packet,
                                     RHI::RHICommandBuffer& cmdBuf)
{
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

    SetupForwardPass(w, h);
}

void ForwardRenderPipeline::SetupForwardPass(uint32_t width, uint32_t height)
{
    RenderPassDesc desc;
    desc.Width  = width;
    desc.Height = height;
    desc.Attachments = { RHI::TextureFormat::RGBA8, RHI::TextureFormat::Depth32 };
    desc.Clear = false;

    RHI::DepthStencilState depthStencil;
    depthStencil.DepthTest  = true;
    depthStencil.DepthWrite = false;
    depthStencil.DepthFunc  = RHI::DepthCompareFunc::Less;

    RHI::RasterizerState rasterizer;
    rasterizer.Cull    = RHI::CullMode::Back;
    rasterizer.Winding = RHI::FrontFace::CCW;
    rasterizer.Fill    = RHI::FillMode::Solid;

    RHI::BlendState blend;
    blend.Enable   = true;
    blend.SrcColor = RHI::BlendFactor::SrcAlpha;
    blend.DstColor = RHI::BlendFactor::OneMinusSrcAlpha;
    blend.ColorOp  = RHI::BlendOp::Add;

    auto& node = m_Graph.AddNode("Forward", desc);
    node.WriteTo("forwardSceneColor");
    node.WriteTo("forwardDepth");

    node.SetExecute([this, depthStencil, rasterizer, blend](RenderGraphContext& ctx) {
        auto& cmd = *ctx.Cmd;
        const auto& packet = ctx.GetPacket();

        auto& lib = ShaderLibrary::Get();
        auto& psoCache = PSOCache::Get();

        // 透明物体按距离降序排序（从远到近绘制）
        auto sorted = packet.Translucent;
        const auto& view = ctx.GetView();
        std::sort(sorted.begin(), sorted.end(),
            [&view](const auto& a, const auto& b) {
                float distA = glm::distance(view.Position, glm::vec3(a.Transform[3]));
                float distB = glm::distance(view.Position, glm::vec3(b.Transform[3]));
                return distA > distB;
            });

        for (auto& snap : sorted)
        {
            if (!snap.MeshSource) continue;

            const ShaderSnippet* snippet = nullptr;
            static ShaderSnippet emptySnippet;
            if (snap.MaterialSource && snap.MaterialSource->Parent)
                snippet = &snap.MaterialSource->Parent->CompiledSnippet;
            if (!snippet || snippet->GLSLCode.empty())
                snippet = &emptySnippet;

            auto shader = lib.GetOrCreatePipelineVariant("forward", *snippet);
            if (!shader) continue;

            PSODesc pso;
            pso.DepthStencil    = depthStencil;
            pso.Rasterizer      = rasterizer;
            pso.FragmentShader  = shader;
            pso.BlendStates.push_back(blend);

            size_t hash = psoCache.GetOrCreate(pso);
            psoCache.Apply(hash, cmd);

            if (snap.MaterialSource)
                snap.MaterialSource->SetMaterialUBO(m_MaterialUBO.get());

            const auto* refl = lib.GetReflection("forward", *snippet);
            if (snap.MaterialSource)
                snap.MaterialSource->RecordBind(cmd, refl ? *refl : ShaderReflection{});

            cmd.SetMat4("u_Model", snap.Transform);
            cmd.DrawIndexed(snap.MeshSource->GetVertexArray(), snap.MeshSource->GetIndexCount());
        }
    });
}

} // namespace AF
