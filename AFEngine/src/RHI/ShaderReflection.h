#pragma once

// ShaderReflection —— 着色器反射信息
//
// 存储着色器的 uniform 位置、uniform 块绑定和纹理槽位映射。
// 由 GLShader 在编译时收集，MaterialParameterStore::UploadTo 据此进行绑定。

#include <unordered_map>
#include <string>
#include <cstdint>

namespace AF {

struct ShaderReflection
{
    std::unordered_map<std::string, int32_t> UniformLocations;
    std::unordered_map<std::string, uint32_t> UniformBlockBindings;
    std::unordered_map<std::string, uint32_t> TextureBindings;

    uint32_t GetTextureBinding(const std::string& name) const
    {
        auto it = TextureBindings.find(name);
        return (it != TextureBindings.end()) ? it->second : UINT32_MAX;
    }

    int32_t GetUniformLocation(const std::string& name) const
    {
        auto it = UniformLocations.find(name);
        return (it != UniformLocations.end()) ? it->second : -1;
    }
};

} // namespace AF
