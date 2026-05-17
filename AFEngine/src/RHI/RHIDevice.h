#pragma once

// RHIDevice —— 渲染设备抽象接口
//
// 提供最底层的 GPU 操作：视口设置、清屏、索引绘制。
// 工厂 Create 根据平台宏返回对应后端实例（当前仅 GLDevice）。

#include "Core/Types.h"
#include "RHI/RHIVertexArray.h"

#include <glm/glm.hpp>

namespace AF {
namespace RHI {

class RHIDevice : public NonCopyable
{
public:
    virtual ~RHIDevice() = default;

    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(const glm::vec4& color) = 0;
    virtual void Clear() = 0;

    virtual void DrawIndexed(const Ref<RHIVertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

    static Unique<RHIDevice> Create();
};

} // namespace RHI
} // namespace AF
