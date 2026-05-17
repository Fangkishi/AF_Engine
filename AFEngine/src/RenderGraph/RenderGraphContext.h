#pragma once

// RenderGraphContext —— 渲染图执行上下文
//
// 在 Execute(Node) 时传递给每个 Pass 的回调函数。
// 提供 Cmd（命令缓冲区指针）、GetView()（相机视图数据）、
// GetPacket()（实体快照 + 光源数据）、GetInput()（按 ID 或名称获取输入纹理）、
// GetPassTextureSlots()（Compile 阶段分配的 Pass 纹理绑定列表）。

#include "Core/Types.h"
#include "RHI/RHITexture.h"
#include "RHI/RHICommandBuffer.h"
#include "Renderer/RenderView.h"
#include "Renderer/RenderPacket.h"
#include "ShaderVariant/ShaderTemplate.h"

#include <cstdint>
#include <string>
#include <vector>

namespace AF {

class RenderGraph;

using RenderResource = uint32_t;
constexpr RenderResource NullResource = UINT32_MAX;

class RenderGraphContext
{
public:
    const RenderView& GetView() const;
    const RenderPacket& GetPacket() const;

    Ref<RHI::RHITexture2D> GetInput(RenderResource id) const;
    Ref<RHI::RHITexture2D> GetInput(const std::string& name) const;

    /// 当前 Pass 已分配的纹理绑定列表（用于传给 ShaderLibrary 编译着色器）
    const std::vector<TextureBinding>& GetPassTextureSlots() const;

    /// 命令缓冲区指针，Pass 通过它录制绘制命令
    RHI::RHICommandBuffer* Cmd = nullptr;

private:
    friend class RenderGraph;

    const RenderView*   m_View   = nullptr;
    const RenderPacket* m_Packet = nullptr;
    const RenderGraph*  m_Graph  = nullptr;
    const std::vector<TextureBinding>* m_PassBindings = nullptr;
};

} // namespace AF
