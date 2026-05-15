#pragma once

#include "Renderer/RenderPipeline.h"
#include "RHI/RHIUniformBuffer.h"

#include <glm/glm.hpp>

namespace AF {

struct CameraGPU
{
    glm::mat4 ViewProjection        = glm::mat4(1.0f);
    glm::mat4 InverseViewProjection = glm::mat4(1.0f);
    glm::mat4 Projection            = glm::mat4(1.0f);
    glm::vec3 Position              = {};
    float     _pad0                 = 0.0f;
    glm::vec2 ScreenSize            = {};
    float     _pad1[2]              = {};
};

class DeferredRenderPipeline : public RenderPipeline
{
public:
    void OnSetup(Engine& engine) override;
    void OnRender(const RenderView& view, const RenderPacket& packet,
                  RHI::RHICommandBuffer& cmdBuf) override;

    Ref<RHI::RHITexture2D> GetOutput(const std::string& name) const { return m_Graph.GetResourceTexture(name); }
    void Invalidate() { m_Graph.Invalidate(); }

private:
    void SetupGBufferPass(uint32_t width, uint32_t height,
        RenderResource gAlbedo, RenderResource gNormal,
        RenderResource gMaterial, RenderResource gDepth);
    void SetupLightingPass(uint32_t width, uint32_t height,
        RenderResource gAlbedo, RenderResource gNormal,
        RenderResource gMaterial, RenderResource gDepth, RenderResource lightAccum);
    void SetupCompositePass(uint32_t width, uint32_t height,
        RenderResource lightAccum, RenderResource outputTarget);

    Ref<RHI::RHIShader> m_GBufferShader;
    Ref<RHI::RHIShader> m_LightingShader;
    Ref<RHI::RHIShader> m_CompositeShader;
    Ref<Mesh> m_FullscreenQuad;

    Ref<RHI::RHIUniformBuffer> m_CameraUBO;
    bool m_HadLightLastFrame = true;
};

} // namespace AF
