#pragma once

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
    RenderResource ImportTexture(const std::string& name, Ref<RHI::RHITexture2D> texture);
    RenderResource CreateTexture(const std::string& name);

    RenderNode& AddNode(const std::string& name, const RenderPassDesc& desc = {});
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
