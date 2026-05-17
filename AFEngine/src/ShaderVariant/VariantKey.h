#pragma once

// VariantKey —— 着色器变体键
//
// 用于标记着色器的不同变体（如 USE_BASE_TEXTURE、ENABLE_NORMAL_MAP 等）。
// 预留 BakeVariants 使用。

#include <cstdint>
#include <string>
#include <vector>

namespace AF {

struct VariantKey
{
    std::string Name;
    bool        Enabled = false;

    VariantKey() = default;
    explicit VariantKey(const std::string& name) : Name(name), Enabled(false) {}
};

} // namespace AF
