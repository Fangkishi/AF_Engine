#pragma once

#include "Core/System.h"
#include "RenderGraph/RenderGraph.h"
#include "RHI/RHICommandBuffer.h"

namespace AF {

class RenderPipeline : public System
{
public:
    void OnInitialize(Engine& engine) override final;
    void OnUpdate(float dt) override final;

protected:
    virtual void OnSetup(Engine& engine) = 0;
    virtual void OnRender(const RenderView& view, const RenderPacket& packet,
                          RHI::RHICommandBuffer& cmdBuf) = 0;
    RenderGraph m_Graph;
};

} // namespace AF
