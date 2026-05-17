#pragma once

// ForwardRenderPipeline —— 前向渲染管线
//
// 用于半透明物体的 sorted 前向渲染，按相机距离由远到近排序。
// 支持材质变体动态编译。

#include "Renderer/RenderPipeline.h"
#include "RHI/RHIUniformBuffer.h"

namespace AF {

class ForwardRenderPipeline : public RenderPipeline
{
protected:
    void OnSetup(Engine& engine) override;
    void OnRender(const RenderView& view, const RenderPacket& packet,
                  RHI::RHICommandBuffer& cmdBuf) override;

    std::vector<MaterialDomain> GetSupportedDomains() const override
    { return { MaterialDomain::Surface }; }

private:
    void SetupForwardPass(uint32_t width, uint32_t height);

    Ref<Mesh> m_FullscreenQuad;
    Ref<RHI::RHIUniformBuffer> m_CameraUBO;
    Ref<RHI::RHIUniformBuffer> m_MaterialUBO;
};

} // namespace AF
