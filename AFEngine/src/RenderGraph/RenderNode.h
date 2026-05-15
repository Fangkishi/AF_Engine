#pragma once

#include "RHI/RHIFramebuffer.h"
#include "RenderGraph/RenderPassDesc.h"
#include "RenderGraph/RenderGraphContext.h"

#include <functional>
#include <string>
#include <vector>

namespace AF {

class RenderNode
{
public:
    RenderNode(const std::string& name, const RenderPassDesc& desc);

    const std::string& GetName() const { return m_Name; }
    const RenderPassDesc& GetDesc() const { return m_Desc; }

    void Read(RenderResource resId)  { m_Reads.push_back(resId); }
    void Write(RenderResource resId) { m_Writes.push_back({ resId, static_cast<uint32_t>(m_Writes.size()) }); }

    const auto& GetReads() const { return m_Reads; }
    const auto& GetWrites() const { return m_Writes; }

    using ExecuteFn = std::function<void(RenderGraphContext& ctx)>;
    void SetExecute(ExecuteFn fn) { m_Execute = std::move(fn); }
    void Execute(RenderGraphContext& ctx) const { if (m_Execute) m_Execute(ctx); }

    bool HasOutput() const { return m_Framebuffer != nullptr; }
    RHI::RHIFramebuffer& GetFramebuffer() { return *m_Framebuffer; }
    RHI::RHIFramebuffer* GetFramebufferPtr() { return m_Framebuffer.get(); }

private:
    std::string m_Name;
    RenderPassDesc m_Desc;
    ExecuteFn m_Execute;

    Unique<RHI::RHIFramebuffer> m_Framebuffer;

    struct WriteEntry { RenderResource Id; uint32_t Slot; };
    std::vector<RenderResource> m_Reads;
    std::vector<WriteEntry> m_Writes;

    std::vector<RenderNode*> m_Dependents;
    int m_RefCount = 0;

    friend class RenderGraph;
};

} // namespace AF
