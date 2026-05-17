#pragma once

// RHIStorageBuffer —— Storage Buffer Object 抽象接口
//
// 比 UBO 更大（可到 128MB），用于传递大量数据（如光源列表）。
// 对应 OpenGL 的 Shader Storage Buffer Object（SSBO）。

#include "Core/Types.h"

#include <cstdint>

namespace AF {
namespace RHI {

class RHIStorageBuffer : public NonCopyable
{
public:
    virtual ~RHIStorageBuffer() = default;

    virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
    virtual void Bind(uint32_t binding) = 0;

    virtual uint32_t GetRendererID() const = 0;

    static Ref<RHIStorageBuffer> Create(uint32_t size, uint32_t binding = 0);
};

} // namespace RHI
} // namespace AF
