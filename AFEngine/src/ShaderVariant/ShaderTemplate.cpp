#include "ShaderTemplate.h"
#include "MaterialGraph/MaterialCompiler.h"
#include "MaterialGraph/MaterialPin.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_set>

namespace AF {

// 构造：从文件路径加载着色器模板源文本
ShaderTemplate::ShaderTemplate(const std::string& filepath)
	: m_Filepath(filepath)
{
	std::ifstream file(filepath);
	if (file.is_open())
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		m_Source = buffer.str();
	}
}

// 生成材质参数 UBO 结构体和 MaterialParams 结构体的 GLSL 代码
// uboBinding = 1（固定，不与 Pass 纹理冲突）
static std::string GenerateMaterialParamsStructGLSL(const std::vector<MaterialParameterDesc>& snippetParams)
{
	std::unordered_set<std::string> seen;

	auto addDesc = [&](std::vector<MaterialParameterDesc>& out, const MaterialParameterDesc& d) {
		if (seen.count(d.Name)) return;
		auto info = GetStd140Info(d.Type);
		if (info.GlslType.empty()) return;
		seen.insert(d.Name);
		out.push_back(d);
	};

	std::vector<MaterialParameterDesc> allDescs;

	// 材质图暴露的参数（由 MaterialOutputNode::GenerateCode 通过 CollectParameter 产生）
	for (const auto& d : snippetParams)
		addDesc(allDescs, d);

	std::stringstream ss;

	// UBO 声明（binding = 1 固定）
	ss << "layout(std140, binding = 1) uniform MaterialParamsUBO {\n";
	bool hasUboFields = false;
	for (const auto& d : allDescs)
	{
		auto info = GetStd140Info(d.Type);
		std::string uboType = info.IsVec3 ? "vec4" : info.GlslType;
		ss << "    " << uboType << " Param_" << d.Name << ";\n";
		hasUboFields = true;
	}
	if (!hasUboFields)
		ss << "    int _dummy;\n";
	ss << "} materialUBO;\n\n";

	// Struct
	ss << "struct MaterialParams {\n";
	for (const auto& d : allDescs)
	{
		auto info = GetStd140Info(d.Type);
		ss << "    " << info.GlslType << " " << d.Name << ";\n";
	}
	ss << "};\n\n";

	// FromUniforms
	ss << "MaterialParams MaterialParams_FromUniforms() {\n";
	ss << "    MaterialParams p;\n";
	for (const auto& d : allDescs)
	{
		auto info = GetStd140Info(d.Type);
		std::string rhs = "materialUBO.Param_" + d.Name;
		if (info.IsVec3) rhs += ".rgb";
		ss << "    p." << d.Name << " = " << rhs << ";\n";
	}
	ss << "    return p;\n";
	ss << "}\n";

	return ss.str();
}

// 自动分配材质纹理绑定槽位
// 从 passBindings 最大 binding + 1 开始分配
std::vector<TextureBinding> AutoAllocateBindings(const ShaderSnippet& snippet,
                                                  const std::vector<TextureBinding>& passBindings)
{
	uint32_t maxBinding = 1; // material UBO 固定 binding=1
	for (const auto& pb : passBindings)
		maxBinding = std::max(maxBinding, pb.Binding);

	uint32_t nextBinding = maxBinding + 1;
	std::vector<TextureBinding> bindings;
	for (const auto& tex : snippet.Textures)
	{
		bindings.push_back({ tex.Name, nextBinding });
		nextBinding++;
	}

	return bindings;
}

// 计算一组纹理绑定的哈希值
size_t HashTextureBindings(const std::vector<TextureBinding>& bindings)
{
	size_t h = 0;
	for (const auto& b : bindings)
	{
		h ^= std::hash<std::string>{}(b.Name) ^ (static_cast<size_t>(b.Binding) << 1);
	}
	return h;
}

// 替换模板中的占位符（仅第一个匹配项）
void ShaderTemplate::ReplacePlaceholder(std::string& source,
                                        const std::string& placeholder,
                                        const std::string& replacement)
{
	size_t pos = source.find(placeholder);
	if (pos != std::string::npos)
	{
		source.replace(pos, placeholder.length(), replacement);
	}
}

// 处理模板：替换所有占位符生成最终 GLSL 代码
std::string ShaderTemplate::ProcessTemplate(const ShaderSnippet& snippet,
                                            VariantKey variantKey,
                                            const std::vector<TextureBinding>& passBindings,
                                            const std::string& brdfGLSL,
                                            const std::string& snippetGLSL) const
{
	std::string result = m_Source;

	// 替换材质参数 UBO（binding = 1 固定）
	std::string paramsStruct = GenerateMaterialParamsStructGLSL(snippet.Parameters);
	ReplacePlaceholder(result, "@MATERIAL_PARAMS_STRUCT@", paramsStruct);

	ReplacePlaceholder(result, "@COMPILER_BINDING_MATERIAL_PARAMS@", "compiler auto-binding");

	// 生成 Pass 输入纹理声明（@PASS_INPUT_TEXTURES@）
	std::string passDecls;
	for (const auto& b : passBindings)
	{
		passDecls += "layout(binding=" + std::to_string(b.Binding)
		          + ") uniform sampler2D " + b.Name + ";\n";
	}
	ReplacePlaceholder(result, "@PASS_INPUT_TEXTURES@", passDecls);

	// 生成材质纹理声明（@MATERIAL_TEXTURE_DECLARATIONS@）
	auto materialBindings = AutoAllocateBindings(snippet, passBindings);
	std::string materialDecls;
	for (const auto& b : materialBindings)
	{
		materialDecls += "layout(binding=" + std::to_string(b.Binding)
		              + ") uniform sampler2D " + b.Name + ";\n";
	}
	ReplacePlaceholder(result, "@MATERIAL_TEXTURE_DECLARATIONS@", materialDecls);

	ReplacePlaceholder(result, "@MATERIAL_NAME@", snippet.MaterialName);
	ReplacePlaceholder(result, "@VARIANT_KEY@", "0");

	if (!brdfGLSL.empty())
		ReplacePlaceholder(result, "@BRDF_FUNCTION@", brdfGLSL);
	else
		ReplacePlaceholder(result, "@BRDF_FUNCTION@", "// BRDF not specified");

	if (!snippetGLSL.empty())
		ReplacePlaceholder(result, "@MATERIAL_SNIPPET@", snippetGLSL);
	else
		ReplacePlaceholder(result, "@MATERIAL_SNIPPET@", "");

	return result;
}

std::string ShaderTemplate::GetInjectedSource(const ShaderSnippet& snippet,
                                              VariantKey variantKey,
                                              const std::vector<TextureBinding>& passBindings,
                                              const std::string& brdfGLSL,
                                              const std::string& snippetGLSL) const
{
	return ProcessTemplate(snippet, variantKey, passBindings, brdfGLSL, snippetGLSL);
}

} // namespace AF
