#pragma once

// ShaderTemplate —— 着色器模板
//
// 从文件加载 .frag.template / .vert.template 模板，
// 注入 BRDF GLSL、材质片段、纹理绑定信息并替换占位符，
// 生成完整的 GLSL 源码供 ShaderLibrary 编译。

#include <string>
#include <vector>
#include "Core/Types.h"
#include "ShaderVariant/VariantKey.h"

namespace AF {

struct MaterialParameterDesc;
struct TextureSlot;
struct ShaderSnippet;

/// 纹理绑定描述：采样器 uniform 名称 → GLSL binding slot
struct TextureBinding
{
    std::string Name;      // uniform 名称，如 "gAlbedo"
    uint32_t    Binding;   // GLSL binding slot
};

/// 自动分配材质纹理绑定槽位
/// @param passBindings Pass 已占用的纹理槽位（用于计算材质纹理的起始 binding）
std::vector<TextureBinding> AutoAllocateBindings(const ShaderSnippet& snippet,
                                                  const std::vector<TextureBinding>& passBindings);

/// 计算一组 TextureBinding 的哈希值（用于缓存键）
size_t HashTextureBindings(const std::vector<TextureBinding>& bindings);

class ShaderTemplate
{
public:
    explicit ShaderTemplate(const std::string& filepath);

    std::string ProcessTemplate(const ShaderSnippet& snippet,
                                VariantKey variantKey,
                                const std::vector<TextureBinding>& passBindings,
                                const std::string& brdfGLSL = "",
                                const std::string& snippetGLSL = "") const;

    std::string GetInjectedSource(const ShaderSnippet& snippet,
                                  VariantKey variantKey,
                                  const std::vector<TextureBinding>& passBindings,
                                  const std::string& brdfGLSL = "",
                                  const std::string& snippetGLSL = "") const;

    const std::string& GetTemplateSource() const { return m_Source; }

private:
    std::string m_Source;
    std::string m_Filepath;

    static void ReplacePlaceholder(std::string& source,
                                   const std::string& placeholder,
                                   const std::string& replacement);
};

} // namespace AF
