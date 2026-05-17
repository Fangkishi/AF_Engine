#pragma once

// RenderPipeline —— 渲染管线抽象基类
//
// 作为 System 子类，提供标准生命周期：
// - OnInitialize(fixed) → OnSetup(子类实现)
// - OnUpdate(fixed) → 获取 RenderSystem 数据 → OnRender(子类实现) → m_Graph.Execute
//
// 子类只需重写 OnSetup（一次初始化）和 OnRender（每帧组装 RenderGraph）。

#include "Core/System.h"
#include "RenderGraph/RenderGraph.h"
#include "RHI/RHICommandBuffer.h"
#include "Material/MaterialDefines.h"
#include "ShaderVariant/ShaderTemplate.h"

#include <unordered_map>
#include <memory>
#include <vector>

namespace AF {

class RenderPipeline : public System
{
public:
    void OnInitialize(Engine& engine) override final;
    void OnUpdate(float dt) override final;

    virtual std::vector<MaterialDomain> GetSupportedDomains() const { return { MaterialDomain::Surface }; }

protected:
    /// 子类实现：初始化着色器、UBO、网格等
    virtual void OnSetup(Engine& engine) = 0;
    /// 子类实现：用当前帧数据创建命令并组装 RenderGraph
    virtual void OnRender(const RenderView& view, const RenderPacket& packet,
                          RHI::RHICommandBuffer& cmdBuf) = 0;
    RenderGraph m_Graph;

    void RegisterTemplate(const std::string& name, const std::string& filepath);
    ShaderTemplate* GetTemplate(const std::string& name);

private:
    std::unordered_map<std::string, Unique<ShaderTemplate>> m_Templates;
};

} // namespace AF
