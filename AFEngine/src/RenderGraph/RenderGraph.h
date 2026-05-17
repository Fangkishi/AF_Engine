#pragma once

// RenderGraph —— 渲染图
//
// 延迟渲染管线的核心编排器。管线子类在 OnRender 中：
// 1. CreateTexture 声明瞬态资源
// 2. AddNode 注册 Pass（幂等，同名复用）
// 3. SetExecute 为每个 Pass 设置渲染回调
// 4. Execute 自动编译 → 拓扑排序 → 按序回放
//
// Compile 流程：ClearResources → 创建纹理 → BuildAdjacency → TopologicalSort → 创建 FBO

#include "Core/Types.h"
#include "RHI/RHITexture.h"
#include "RHI/RHICommandBuffer.h"
#include "RenderGraph/RenderNode.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace AF {

class RenderGraphContext;

class RenderGraph
{
public:
    /// 注册 Pass 节点（同名幂等，更新 desc）
    RenderNode& AddNode(const std::string& name, const RenderPassDesc& desc = {});
    /// 清空所有节点（光照数量变化等场景使用）
    void ClearNodes();

    RenderResource GetResource(const std::string& name) const;

    void Compile();
    void Execute(const RenderView& view, const RenderPacket& packet, RHI::RHICommandBuffer& cmdBuf);
    void Invalidate();
    bool IsCompiled() const { return m_Compiled; }

    const std::vector<RenderNode*>& GetExecutionOrder() const { return m_ExecutionOrder; }
    Ref<RHI::RHITexture2D> GetTexture(RenderResource id) const;
    Ref<RHI::RHITexture2D> GetResourceTexture(const std::string& name) const { return GetTexture(GetResource(name)); }

private:
    RenderResource ImportTexture(const std::string& name, Ref<RHI::RHITexture2D> texture);
    RenderResource CreateTexture(const std::string& name);

    void BuildAdjacency();
    void TopologicalSort();
    bool HasDependency(const RenderNode& from, const RenderNode& to) const;
    void ClearResources();

    struct ResourceData
    {
        std::string Name;
        Ref<RHI::RHITexture2D> Texture;
        bool IsImported = false;
    };

    std::vector<ResourceData> m_ResourceData;
    std::vector<Unique<RenderNode>> m_Nodes;
    std::vector<RenderNode*> m_ExecutionOrder;
    bool m_Compiled = false;
    mutable std::unordered_map<std::string, RenderResource> m_NameCache;
};

} // namespace AF
