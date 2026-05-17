#pragma once

// RHITexture —— 2D 纹理和 Cube 纹理抽象接口
//
// 提供创建空白纹理、从文件加载纹理及立方体贴图的能力。
// Cube 纹理支持从 6 张单面图片加载。

#include "Core/Types.h"
#include "RHI/RHITypes.h"

#include <array>
#include <cstdint>
#include <string>

namespace AF {
namespace RHI {

class RHITexture2D
{
public:
    virtual ~RHITexture2D() = default;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetRendererID() const = 0;

    virtual void Bind(uint32_t slot = 0) const = 0;

    virtual TextureFormat GetFormat() const = 0;

    /// 创建指定尺寸和格式的空白纹理（常用于渲染目标附件）
    static Ref<RHITexture2D> Create(uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8);
    /// 从图片文件加载
    static Ref<RHITexture2D> Create(const std::string& path);
};

class RHITextureCube
{
public:
    virtual ~RHITextureCube() = default;

    virtual uint32_t GetSize() const = 0;
    virtual uint32_t GetRendererID() const = 0;

    virtual void Bind(uint32_t slot = 0) const = 0;

    virtual TextureFormat GetFormat() const = 0;

    /// 创建指定尺寸的空白立方体贴图
    static Ref<RHITextureCube> Create(uint32_t size, TextureFormat format = TextureFormat::RGBA8);
    /// 从 6 个面图片加载（顺序：+X -X +Y -Y +Z -Z）
    static Ref<RHITextureCube> Create(const std::array<std::string, 6>& facePaths);
};

} // namespace RHI
} // namespace AF
