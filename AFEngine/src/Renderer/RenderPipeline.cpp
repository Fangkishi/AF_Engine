#include "Renderer/RenderPipeline.h"
#include "Renderer/Renderer.h"

#include "Core/Engine.h"
#include "Core/Log.h"

namespace AF {

void RenderPipeline::OnInitialize(Engine& engine)
{
    AF_LOG_INFO("RenderPipeline: initializing...");
    OnSetup(engine);
}

void RenderPipeline::OnUpdate(float dt)
{
    (void)dt;

    auto& rs = GetEngine()->GetSystem<RenderSystem>();
    auto& view   = rs.GetView();
    auto& packet = rs.GetPacket();
    auto& device = GetEngine()->GetDevice();

    RHI::RHICommandBuffer cmdBuf;
    cmdBuf.Begin();

    OnRender(view, packet, cmdBuf);
    m_Graph.Execute(view, packet, cmdBuf);

    cmdBuf.End();
    cmdBuf.Execute(device);
}

} // namespace AF
