#include "MaterialGraph/Nodes/TextureNodes.h"
#include "MaterialGraph/NodeFactory.h"
#include "MaterialGraph/MaterialCompiler.h"

namespace AF {

// ============================================================================
// TextureSampleNode — 纹理采样节点
//
// 输入：Tex(Texture2D), UVs(vec2), Sampler(SamplerState)
// 输出：RGBA(vec4), R/G/B/A(各为 float)
// 同时为每个纹理采样实例生成唯一的变体宏定义，用于 ShaderVariant 系统。
// ============================================================================

TextureSampleNode::TextureSampleNode(NodeID id)
	: MaterialNode(id, "TextureSample")
{
	AddInputPin("Tex", MaterialPinType::Texture2D);
	AddInputPin("UVs", MaterialPinType::Float2, glm::vec2(0.0f));
	AddInputPin("Sampler", MaterialPinType::SamplerState);
	AddOutputPin("RGBA", MaterialPinType::Float4);
	AddOutputPin("R", MaterialPinType::Float);
	AddOutputPin("G", MaterialPinType::Float);
	AddOutputPin("B", MaterialPinType::Float);
	AddOutputPin("A", MaterialPinType::Float);
}

// 为每个纹理采样节点生成唯一的宏定义，用于条件编译
std::vector<std::string> TextureSampleNode::GetVariantDefines() const
{
	return { "_TEXTURESAMPLE_" + std::to_string(m_ID.Value) };
}

std::string TextureSampleNode::GenerateCode(CompilerContext& ctx)
{
	std::string texExpr = ctx.GetInputExpression(m_ID, "Tex");
	std::string uvExpr = ctx.GetInputExpression(m_ID, "UVs");

	// RGBA 输出：texture() 采样结果
	std::string rgbaVar = ctx.AllocateVariable(m_ID, "tex_rgba");
	ctx.EmitLine("vec4 " + rgbaVar + " = texture(" + texExpr + ", " + uvExpr + ");");

	// 分别提取 R/G/B/A 分量
	std::string rVar = ctx.AllocateVariable(m_ID, "tex_r");
	ctx.EmitLine("float " + rVar + " = " + rgbaVar + ".r;");

	std::string gVar = ctx.AllocateVariable(m_ID, "tex_g");
	ctx.EmitLine("float " + gVar + " = " + rgbaVar + ".g;");

	std::string bVar = ctx.AllocateVariable(m_ID, "tex_b");
	ctx.EmitLine("float " + bVar + " = " + rgbaVar + ".b;");

	std::string aVar = ctx.AllocateVariable(m_ID, "tex_a");
	ctx.EmitLine("float " + aVar + " = " + rgbaVar + ".a;");

	return rgbaVar;
}

REGISTER_MATERIAL_NODE(TextureSampleNode, "TextureSample")

} // namespace AF
