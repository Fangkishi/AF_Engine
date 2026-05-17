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

/// 每帧流程：获取 RenderSystem 的数据 → 子类 OnRender 录命令 → Graph.Execute
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

void RenderPipeline::RegisterTemplate(const std::string& name, const std::string& filepath)
{
    m_Templates[name] = std::make_unique<ShaderTemplate>(filepath);
}

ShaderTemplate* RenderPipeline::GetTemplate(const std::string& name)
{
    auto it = m_Templates.find(name);
    return (it != m_Templates.end()) ? it->second.get() : nullptr;
}

} // namespace AF
