#include "ShaderVariant/ShaderLibrary.h"
#include "ShaderVariant/ShaderTemplate.h"
#include "MaterialGraph/MaterialCompiler.h"
#include "Renderer/Brdf/BRDF.h"
#include "RHI/RHIShader.h"
#include "Core/Log.h"

namespace AF {

// 获取着色器库单例实例
ShaderLibrary& ShaderLibrary::Get()
{
    static ShaderLibrary instance;
    return instance;
}

// 对 ShaderSnippet 进行哈希（基于 GLSL 代码字符串）
size_t ShaderLibrary::HashSnippet(const ShaderSnippet& snippet)
{
    return std::hash<std::string>{}(snippet.GLSLCode);
}

// 获取或创建管道变体着色器
Ref<RHI::RHIShader> ShaderLibrary::GetOrCreatePipelineVariant(
    const std::string& templateName,
    const ShaderSnippet& snippet,
    const std::vector<TextureBinding>& passBindings)
{
    CacheKey key{templateName, HashSnippet(snippet), HashTextureBindings(passBindings)};

    // 检查缓存
    auto it = m_Cache.find(key);
    if (it != m_Cache.end())
        return it->second.Shader;

    // 加载片段着色器模板
    std::string templatePath = "assets/shaders/templates/" + templateName + ".frag.template";
    ShaderTemplate shaderTemplate(templatePath);

    if (shaderTemplate.GetTemplateSource().empty())
    {
        AF_LOG_ERROR("ShaderLibrary: Failed to load template '{}'", templatePath);
        return nullptr;
    }

    // 获取默认 BRDF 的 GLSL 代码
    std::string brdfCode;
    if (auto brdf = BRDF::Get("DefaultLit"))
        brdfCode = brdf->GenerateGLSL();

    // 将材质片段注入片段着色器模板（passBindings 用于生成 @PASS_INPUT_TEXTURES@ 并计算材质纹理起始）
    std::string injectedSource = shaderTemplate.GetInjectedSource(snippet, VariantKey{}, passBindings, brdfCode, snippet.GLSLCode);

    // 加载顶点着色器模板
    std::string vertPath = "assets/shaders/templates/" + templateName + ".vert.template";
    ShaderTemplate vertTemplate(vertPath);
    if (vertTemplate.GetTemplateSource().empty())
    {
        AF_LOG_ERROR("ShaderLibrary: Failed to load template '{}'", vertPath);
        return nullptr;
    }
    std::string vertSource = vertTemplate.GetInjectedSource(snippet, VariantKey{}, passBindings);

    // 创建 RHI 着色器对象并编译
    auto shader = RHI::RHIShader::Create(templateName, vertSource, injectedSource);

    // 收集着色器反射信息并缓存
    CompiledShaderVariant compiled;
    compiled.Shader = shader;
    compiled.Reflection = shader->CollectReflection();

    m_Cache[key] = compiled;

    AF_LOG_INFO("ShaderLibrary: Compiled '{}' (snippetHash={}, passHash={})", templateName, key.SnippetHash, key.PassBindingsHash);
    return shader;
}

// 批量烘焙变体（预编译所有变体，当前实现直接调用单变体编译）
void ShaderLibrary::BakeVariants(
    const std::string& templateName,
    const ShaderSnippet& snippet,
    const std::vector<VariantKey>& variants)
{
    (void)variants;
    GetOrCreatePipelineVariant(templateName, snippet);
}

// 获取着色器反射信息
const ShaderReflection* ShaderLibrary::GetReflection(
    const std::string& templateName, const ShaderSnippet& snippet,
    const std::vector<TextureBinding>& passBindings) const
{
    CacheKey key{templateName, HashSnippet(snippet), HashTextureBindings(passBindings)};
    auto it = m_Cache.find(key);
    return (it != m_Cache.end()) ? &it->second.Reflection : nullptr;
}

// 使指定模板的所有缓存变体失效（重新编译）
void ShaderLibrary::InvalidateTemplate(const std::string& templateName)
{
    for (auto it = m_Cache.begin(); it != m_Cache.end(); )
    {
        if (it->first.TemplateName == templateName)
            it = m_Cache.erase(it);
        else
            ++it;
    }
}

} // namespace AF
