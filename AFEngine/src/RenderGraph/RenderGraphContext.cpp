#include "RenderGraph/RenderGraphContext.h"
#include "RenderGraph/RenderGraph.h"

namespace AF {

const RenderView& RenderGraphContext::GetView() const
{
    return *m_View;
}

const RenderPacket& RenderGraphContext::GetPacket() const
{
    return *m_Packet;
}

Ref<RHI::RHITexture2D> RenderGraphContext::GetInput(RenderResource id) const
{
    return m_Graph ? m_Graph->GetTexture(id) : nullptr;
}

Ref<RHI::RHITexture2D> RenderGraphContext::GetInput(const std::string& name) const
{
    if (!m_Graph)
        return nullptr;

    RenderResource id = m_Graph->GetResource(name);
    return (id != NullResource) ? m_Graph->GetTexture(id) : nullptr;
}

} // namespace AF
