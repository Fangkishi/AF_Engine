#pragma once

// DeferredRenderPipeline —— 延迟渲染管线
//
// 内置 RenderGraph 节点：
// 1. GBuffer（4x MRT：Albedo + Normal + Material + Depth）
// 2. Composite（从 gAlbedo 直接复合输出到 finalComposite）
//
// 视口尺寸变化时通过 Invalidate() → m_Graph.Invalidate() 触发重编译。

#include "Renderer/RenderPipeline.h"
#include "Renderer/CameraGPU.h"
#include "RHI/RHIUniformBuffer.h"

namespace AF {

class DeferredRenderPipeline : public RenderPipeline
{
public:
    void OnSetup(Engine& engine) override;
    void OnRender(const RenderView& view, const RenderPacket& packet,
                  RHI::RHICommandBuffer& cmdBuf) override;

    /// 按资源名称获取输出纹理（供 ViewportPanel 显示用）
    Ref<RHI::RHITexture2D> GetOutput(const std::string& name) const { return m_Graph.GetResourceTexture(name); }
    void Invalidate() { m_Graph.Invalidate(); }

private:
    void SetupGBufferPass(uint32_t width, uint32_t height);
    void SetupCompositePass(uint32_t width, uint32_t height);

    Ref<Mesh> m_FullscreenQuad;

    Ref<RHI::RHIUniformBuffer> m_CameraUBO;
    Ref<RHI::RHIUniformBuffer> m_MaterialUBO;
};

} // namespace AF
