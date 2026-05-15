#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphContext.h"

#include "Core/Assert.h"
#include "Core/Log.h"
#include "RHI/RHIFramebuffer.h"

#include <queue>

namespace AF {

RenderNode::RenderNode(const std::string& name, const RenderPassDesc& desc)
    : m_Name(name)
    , m_Desc(desc)
{
}

RenderResource RenderGraph::ImportTexture(const std::string& name, Ref<RHI::RHITexture2D> texture)
{
    auto it = m_NameCache.find(name);
    if (it != m_NameCache.end())
        return it->second;

    RenderResource id = static_cast<RenderResource>(m_ResourceData.size());
    m_ResourceData.push_back({ name, texture, true });
    m_NameCache[name] = id;
    AF_LOG_INFO("RenderGraph: import texture '{}' (id={})", name, id);
    return id;
}

RenderResource RenderGraph::CreateTexture(const std::string& name)
{
    auto it = m_NameCache.find(name);
    if (it != m_NameCache.end())
        return it->second;

    RenderResource id = static_cast<RenderResource>(m_ResourceData.size());
    m_ResourceData.push_back({ name, nullptr, false });
    m_NameCache[name] = id;
    AF_LOG_INFO("RenderGraph: declare texture '{}' (id={})", name, id);
    return id;
}

RenderNode& RenderGraph::AddNode(const std::string& name, const RenderPassDesc& desc)
{
    for (auto& n : m_Nodes)
    {
        if (n->GetName() == name)
        {
            n->m_Reads.clear();
            n->m_Writes.clear();
            n->m_Dependents.clear();
            n->m_RefCount = 0;
            n->m_Desc = desc;
            return *n;
        }
    }

    auto node = std::make_unique<RenderNode>(name, desc);
    RenderNode& ref = *node;
    m_Nodes.push_back(std::move(node));
    m_Compiled = false;
    AF_LOG_INFO("RenderGraph: add node '{}'", name);
    return ref;
}

void RenderGraph::ClearNodes()
{
    AF_LOG_INFO("RenderGraph: clearing {} nodes", m_Nodes.size());
    m_Nodes.clear();
    m_ExecutionOrder.clear();
    m_Compiled = false;
}

RenderResource RenderGraph::GetResource(const std::string& name) const
{
    auto it = m_NameCache.find(name);
    return (it != m_NameCache.end()) ? it->second : NullResource;
}

bool RenderGraph::HasDependency(const RenderNode& from, const RenderNode& to) const
{
    for (auto& w : from.GetWrites())
        for (auto r : to.GetReads())
            if (w.Id == r) return true;

    for (auto& w1 : from.GetWrites())
        for (auto& w2 : to.GetWrites())
            if (w1.Id == w2.Id) return true;

    return false;
}

void RenderGraph::BuildAdjacency()
{
    for (auto& node : m_Nodes)
    {
        node->m_Dependents.clear();
        node->m_RefCount = 0;
    }

    for (size_t i = 0; i < m_Nodes.size(); ++i)
    {
        for (size_t j = 0; j < m_Nodes.size(); ++j)
        {
            if (i == j) continue;
            if (HasDependency(*m_Nodes[i], *m_Nodes[j]))
            {
                m_Nodes[i]->m_Dependents.push_back(m_Nodes[j].get());
                m_Nodes[j]->m_RefCount++;
            }
        }
    }
}

void RenderGraph::TopologicalSort()
{
    m_ExecutionOrder.clear();
    std::queue<RenderNode*> q;

    for (auto& node : m_Nodes)
        if (node->m_RefCount == 0)
            q.push(node.get());

    while (!q.empty())
    {
        RenderNode* node = q.front();
        q.pop();
        m_ExecutionOrder.push_back(node);

        for (auto* dep : node->m_Dependents)
            if (--dep->m_RefCount == 0)
                q.push(dep);
    }
}

void RenderGraph::ClearResources()
{
    for (auto& node : m_Nodes)
        node->m_Framebuffer.reset();
    for (auto& res : m_ResourceData)
        if (!res.IsImported)
            res.Texture.reset();
}

void RenderGraph::Compile()
{
    if (m_Compiled)
    {
        AF_LOG_INFO("RenderGraph: already compiled, skipping");
        return;
    }

    AF_LOG_INFO("RenderGraph: compiling {} nodes...", m_Nodes.size());

    ClearResources();

    for (auto& node : m_Nodes)
    {
        for (auto& w : node->GetWrites())
        {
            if (w.Id >= m_ResourceData.size()) continue;
            auto& res = m_ResourceData[w.Id];
            if (res.IsImported) continue;

            if (w.Slot >= node->m_Desc.Attachments.size()) continue;
            auto fmt = node->m_Desc.Attachments[w.Slot];

            if (!res.Texture)
            {
                res.Texture = RHI::RHITexture2D::Create(node->m_Desc.Width, node->m_Desc.Height, fmt);
                AF_LOG_INFO("RenderGraph: create transient texture '{}' format={} size={}x{}",
                    res.Name, static_cast<int>(fmt), node->m_Desc.Width, node->m_Desc.Height);
            }
            else
            {
                AF_CORE_ASSERT(res.Texture->GetFormat() == fmt);
            }
        }
    }

    BuildAdjacency();
    TopologicalSort();

    for (auto& node : m_Nodes)
    {
        if (node->GetWrites().empty()) continue;

        node->m_Framebuffer = RHI::RHIFramebuffer::Create();

        for (auto& w : node->GetWrites())
        {
            if (w.Id >= m_ResourceData.size()) continue;
            auto& res = m_ResourceData[w.Id];
            if (!res.Texture) continue;

            if (IsDepthFormat(res.Texture->GetFormat()))
                node->m_Framebuffer->AttachDepth(res.Texture);
            else
                node->m_Framebuffer->AttachColor(res.Texture, w.Slot);
        }
    }

    AF_LOG_INFO("RenderGraph: compiled -- {} nodes in execution order", m_ExecutionOrder.size());

    m_Compiled = true;
}

void RenderGraph::Invalidate()
{
    AF_LOG_INFO("RenderGraph: invalidated");
    m_Compiled = false;
}

void RenderGraph::Execute(const RenderView& view, const RenderPacket& packet, RHI::RHICommandBuffer& cmdBuf)
{
    if (!m_Compiled)
        Compile();

    for (auto* node : m_ExecutionOrder)
    {
        if (node->HasOutput())
            cmdBuf.BindFramebuffer(node->GetFramebufferPtr());

        RenderGraphContext ctx;
        ctx.m_View   = &view;
        ctx.m_Packet = &packet;
        ctx.m_Graph  = this;
        ctx.Cmd      = &cmdBuf;

        node->Execute(ctx);

        if (node->HasOutput())
            cmdBuf.UnbindFramebuffer(node->GetFramebufferPtr());
    }
}

Ref<RHI::RHITexture2D> RenderGraph::GetTexture(RenderResource id) const
{
    if (id >= m_ResourceData.size()) return nullptr;
    return m_ResourceData[id].Texture;
}

} // namespace AF
