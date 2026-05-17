#pragma once

// ShaderLibrary —— 着色器变体库（单例）
//
// 根据 ShaderTemplate + ShaderSnippet + PassTextureBindings 动态编译着色器变体。
// 缓存已编译的变体，避免重复编译。
// InvalidateTemplate 可在模板文件变更时清空缓存。

#include "Core/Types.h"
#include "ShaderVariant/VariantKey.h"
#include "ShaderVariant/ShaderTemplate.h"
#include "RHI/RHIShader.h"
#include "RHI/ShaderReflection.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace AF {

struct ShaderSnippet;

struct CompiledShaderVariant
{
    Ref<RHI::RHIShader> Shader;
    ShaderReflection    Reflection;
};

class ShaderLibrary
{
public:
    static ShaderLibrary& Get();

    /// 获取或创建管道变体着色器
    /// @param passBindings 当前 Pass 的输入纹理绑定（用于生成 @PASS_INPUT_TEXTURES@ 并计算材质纹理起始 binding）
    Ref<RHI::RHIShader> GetOrCreatePipelineVariant(
        const std::string& templateName,
        const ShaderSnippet& snippet,
        const std::vector<TextureBinding>& passBindings = {}
    );

    void BakeVariants(
        const std::string& templateName,
        const ShaderSnippet& snippet,
        const std::vector<VariantKey>& variants
    );

    const ShaderReflection* GetReflection(const std::string& templateName,
                                            const ShaderSnippet& snippet,
                                            const std::vector<TextureBinding>& passBindings = {}) const;

    void InvalidateTemplate(const std::string& templateName);

private:
    struct CacheKey
    {
        std::string TemplateName;
        size_t      SnippetHash    = 0;
        size_t      PassBindingsHash = 0;
        bool operator==(const CacheKey& other) const
        { return TemplateName == other.TemplateName && SnippetHash == other.SnippetHash && PassBindingsHash == other.PassBindingsHash; }
    };

    struct CacheKeyHash
    {
        size_t operator()(const CacheKey& key) const
        {
            return std::hash<std::string>{}(key.TemplateName) ^ key.SnippetHash ^ key.PassBindingsHash;
        }
    };

    std::unordered_map<CacheKey, CompiledShaderVariant, CacheKeyHash> m_Cache;

    static size_t HashSnippet(const ShaderSnippet& snippet);
};

} // namespace AF
