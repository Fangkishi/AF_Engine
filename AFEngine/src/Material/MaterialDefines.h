#pragma once

// MaterialDefines —— 材质枚举定义

#include <cstdint>

namespace AF {

enum class MaterialDomain : uint8_t { Surface, PostProcess, Decal };
enum class MaterialBlendMode : uint8_t { Opaque, Masked, Translucent };

} // namespace AF
