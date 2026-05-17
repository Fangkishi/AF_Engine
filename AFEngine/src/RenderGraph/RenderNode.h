#pragma once

// RenderNode —— 渲染图节点
//
// 每个节点代表一个渲染 Pass。WriteTo/ReadFrom 声明输入/输出资源，
// 字符串参数在 Compile 时由 RenderGraph 解析为实际纹理。
// 节点间通过读写声明自动建立依赖拓扑。
// ReadFrom 声明的资源自动分配 binding slot 并在 Execute 时自动绑定。
// Import 注册外部纹理，后续 WriteTo/ReadFrom 通过同名自动关联。

#include "RHI/RHIFramebuffer.h"
#include "RenderGraph/RenderPassDesc.h"
#include "RenderGraph/RenderGraphContext.h"
#include "ShaderVariant/ShaderTemplate.h"

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

    /// 注册外部纹理（后续 WriteTo/ReadFrom 同名自动关联，Compile 时通过 ImportTexture 接入）
    void Import(const std::string& name, Ref<RHI::RHITexture2D> texture);
    /// 声明写入资源（Compile 时自动 CreateTexture + FBO 附着，按调用顺序分配 slot）
    void WriteTo(const std::string& name);
    /// 声明读取资源（自动分配 binding slot + 自动绑定，sampler uniform 名默认 = 资源名）
    void ReadFrom(const std::string& name);
    /// 声明读取资源并指定 sampler uniform 名称
    void ReadFrom(const std::string& name, const std::string& samplerName);

    struct ImportEntry { std::string Name; Ref<RHI::RHITexture2D> Texture; };
    struct WriteEntry  { std::string Name; RenderResource Id = NullResource; uint32_t Slot = 0; };
    struct ReadEntry   { std::string Name; std::string SamplerName; RenderResource Id = NullResource; uint32_t AllocatedSlot = 0; };

    const auto& GetReads() const { return m_Reads; }
    const auto& GetWrites() const { return m_Writes; }
    const auto& GetImports() const { return m_Imports; }

    const auto& GetPassBindings() const { return m_PassBindings; }

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

    std::vector<ImportEntry> m_Imports;
    std::vector<ReadEntry> m_Reads;
    std::vector<WriteEntry> m_Writes;
    std::vector<TextureBinding> m_PassBindings;

    std::vector<RenderNode*> m_Dependents;
    int m_RefCount = 0;

    friend class RenderGraph;
};

} // namespace AF
