#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphContext.h"

#include "Core/Assert.h"
#include "Core/Log.h"
#include "RHI/RHIFramebuffer.h"

#include <queue>
#include <unordered_set>

namespace AF {

// ── RenderNode ──

RenderNode::RenderNode(const std::string& name, const RenderPassDesc& desc)
    : m_Name(name)
    , m_Desc(desc)
{
}

void RenderNode::Import(const std::string& name, Ref<RHI::RHITexture2D> texture)
{
    for (auto& imp : m_Imports)
        if (imp.Name == name) return;
    m_Imports.push_back({ name, std::move(texture) });
}

void RenderNode::WriteTo(const std::string& name)
{
    for (auto& w : m_Writes)
        if (w.Name == name) return;
    m_Writes.push_back({ name, NullResource, static_cast<uint32_t>(m_Writes.size()) });
}

void RenderNode::ReadFrom(const std::string& name)
{
    for (auto& r : m_Reads)
        if (r.Name == name) return;
    m_Reads.push_back({ name, "", NullResource, 0 });
}

void RenderNode::ReadFrom(const std::string& name, const std::string& samplerName)
{
    for (auto& r : m_Reads)
        if (r.Name == name) return;
    m_Reads.push_back({ name, samplerName, NullResource, 0 });
}

// ── 资源管理 ──

/// 注册一个外部导入的纹理（不被 ClearResources 销毁）
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

/// 声明一个瞬态纹理（在 Compile 时由 RenderGraph 自动创建）
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

// ── 节点管理 ──

RenderNode& RenderGraph::AddNode(const std::string& name, const RenderPassDesc& desc)
{
    for (auto& n : m_Nodes)
    {
        if (n->GetName() == name)
        {
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

// ── 依赖图构建 ──

bool RenderGraph::HasDependency(const RenderNode& from, const RenderNode& to) const
{
    // 写-读依赖：from 写出的资源被 to 读取
    for (auto& w : from.GetWrites())
        for (auto& r : to.GetReads())
            if (w.Id == r.Id) return true;

    // 写-写依赖：from 和 to 写入同一资源
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

// ── 拓扑排序（Kahn 算法）──

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

// ── 资源清理 ──

void RenderGraph::ClearResources()
{
    for (auto& node : m_Nodes)
        node->m_Framebuffer.reset();
    for (auto& res : m_ResourceData)
        if (!res.IsImported)
            res.Texture.reset();
}

// ── 编译 ──

void RenderGraph::Compile()
{
    if (m_Compiled)
    {
        AF_LOG_INFO("RenderGraph: already compiled, skipping");
        return;
    }

    AF_LOG_INFO("RenderGraph: compiling {} nodes...", m_Nodes.size());

    // 0. 处理所有 Import 声明 — 将外部纹理注册到图
    for (auto& node : m_Nodes)
    {
        for (auto& imp : node->m_Imports)
        {
            ImportTexture(imp.Name, imp.Texture);
        }
    }

    // 1. 清空旧资源和 FBO
    ClearResources();

    // 2. 名称解析 — WriteTo → CreateTexture
    for (auto& node : m_Nodes)
    {
        for (auto& w : node->m_Writes)
        {
            w.Id = CreateTexture(w.Name);
        }
    }

    // 3. 名称解析 — ReadFrom → GetResource（先查 Import，再查 WriteTo 创建的，都不到则 assert）
    //    同时去重（ReadFrom 幂等在声明时防止同一帧重复，此处消除跨帧残留）
    for (auto& node : m_Nodes)
    {
        std::unordered_set<std::string> seen;
        std::vector<RenderNode::ReadEntry> unique;
        for (auto& r : node->m_Reads)
        {
            if (!seen.insert(r.Name).second) continue;
            unique.push_back(r);
        }
        node->m_Reads = std::move(unique);

        for (auto& r : node->m_Reads)
        {
            r.Id = GetResource(r.Name);
            if (r.Id == NullResource)
            {
                AF_CORE_ASSERT(false, "RenderGraph::Compile: ReadFrom resource not declared (no WriteTo or ImportTexture)");
            }

            // 默认 sampler uniform 名 = 资源名
            if (r.SamplerName.empty())
                r.SamplerName = r.Name;
        }
    }

    // 4. 根据每个节点的写入声明创建瞬态纹理
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

    // 5. 构建依赖图并拓扑排序
    BuildAdjacency();
    TopologicalSort();

    // 6. 为每个节点的 ReadFrom 分配纹理绑定槽位
    //    binding 从 2 开始（0 = Camera UBO, 1 = Material UBO）
    uint32_t globalNextSlot = 2;
    for (auto& node : m_Nodes)
    {
        node->m_PassBindings.clear();

        for (auto& r : node->m_Reads)
        {
            r.AllocatedSlot = globalNextSlot;
            node->m_PassBindings.push_back({ r.SamplerName, globalNextSlot });
            AF_LOG_INFO("RenderGraph: node '{}' bind sampler '{}' -> slot {}",
                node->GetName(), r.SamplerName, globalNextSlot);

            globalNextSlot++;
        }
    }

    // 7. 为每个节点创建 FBO（附着其写入的纹理）
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

// ── 执行 ──

void RenderGraph::Execute(const RenderView& view, const RenderPacket& packet, RHI::RHICommandBuffer& cmdBuf)
{
    if (!m_Compiled)
        Compile();

    for (auto* node : m_ExecutionOrder)
    {
        const auto& desc = node->GetDesc();

        cmdBuf.SetViewport(0, 0, desc.Width, desc.Height);

        if (node->HasOutput())
            cmdBuf.BindFramebuffer(node->GetFramebufferPtr());

        if (desc.Clear)
        {
            cmdBuf.PushDepthMask();
            cmdBuf.SetClearColor(desc.ClearColor);
            cmdBuf.Clear();
            cmdBuf.PopDepthMask();
        }

        // 自动绑定 Pass 输入纹理
        for (auto& bind : node->m_PassBindings)
        {
            auto texture = GetResourceTexture(bind.Name);
            if (texture)
                cmdBuf.BindTexture(bind.Binding, texture);
        }

        RenderGraphContext ctx;
        ctx.m_View         = &view;
        ctx.m_Packet       = &packet;
        ctx.m_Graph        = this;
        ctx.Cmd            = &cmdBuf;
        ctx.m_PassBindings = &node->m_PassBindings;

        node->Execute(ctx);

        if (node->HasOutput())
            cmdBuf.UnbindFramebuffer(node->GetFramebufferPtr());
    }
}

// ── 纹理查询 ──

Ref<RHI::RHITexture2D> RenderGraph::GetTexture(RenderResource id) const
{
    if (id >= m_ResourceData.size()) return nullptr;
    return m_ResourceData[id].Texture;
}

} // namespace AF
