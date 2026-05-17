#include "MaterialGraph/Nodes/VectorNodes.h"
#include "MaterialGraph/NodeFactory.h"
#include "MaterialGraph/MaterialCompiler.h"

namespace AF {

// ============================================================================
// DotProductNode — dot(A, B)
// ============================================================================

DotProductNode::DotProductNode(NodeID id)
	: MaterialNode(id, "DotProduct")
{
	AddInputPin("A", MaterialPinType::Float4, 0.0f);
	AddInputPin("B", MaterialPinType::Float4, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float);  // 点积结果始终为标量
}

std::string DotProductNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "dot");

	ctx.EmitLine("float " + varName + " = dot(" + exprA + ", " + exprB + ");");
	return varName;
}

// ============================================================================
// CrossProductNode — cross(A, B)
// 仅支持 vec3 输入
// ============================================================================

CrossProductNode::CrossProductNode(NodeID id)
	: MaterialNode(id, "CrossProduct")
{
	AddInputPin("A", MaterialPinType::Float3, glm::vec3(0.0f));
	AddInputPin("B", MaterialPinType::Float3, glm::vec3(0.0f));
	AddOutputPin("Out", MaterialPinType::Float3);
}

std::string CrossProductNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "cross");

	ctx.EmitLine("vec3 " + varName + " = cross(" + exprA + ", " + exprB + ");");
	return varName;
}

// ============================================================================
// NormalizeNode — normalize(In)
// ============================================================================

NormalizeNode::NormalizeNode(NodeID id)
	: MaterialNode(id, "Normalize")
{
	AddInputPin("In", MaterialPinType::Float4, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string NormalizeNode::GenerateCode(CompilerContext& ctx)
{
	std::string expr = ctx.GetInputExpression(m_ID, "In");
	std::string varName = ctx.AllocateVariable(m_ID, "norm");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = normalize(" + expr + ");");
	return varName;
}

// ============================================================================
// AppendVectorNode — vec3(A, B)
// 将 vec2(A) 和 float(B) 拼接为 vec3
// ============================================================================

AppendVectorNode::AppendVectorNode(NodeID id)
	: MaterialNode(id, "AppendVector")
{
	AddInputPin("A", MaterialPinType::Float2, glm::vec2(0.0f));
	AddInputPin("B", MaterialPinType::Float, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float3);
}

std::string AppendVectorNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "append");

	ctx.EmitLine("vec3 " + varName + " = vec3(" + exprA + ", " + exprB + ");");
	return varName;
}

// ============================================================================
// ComponentMaskNode — 按 RGBA 掩码提取向量分量
// 掩码字符串如 "RGB"、"R"、"RGBA"，大小写均可
// 输出类型根据掩码长度自动推断
// ============================================================================

ComponentMaskNode::ComponentMaskNode(NodeID id, const std::string& mask)
	: MaterialNode(id, "ComponentMask")
{
	Properties["Mask"] = mask;

	AddInputPin("In", MaterialPinType::Float4, glm::vec4(0.0f));

	// 根据掩码长度确定输出类型
	if (mask.length() == 1)
		AddOutputPin("Out", MaterialPinType::Float);
	else if (mask.length() == 2)
		AddOutputPin("Out", MaterialPinType::Float2);
	else if (mask.length() == 3)
		AddOutputPin("Out", MaterialPinType::Float3);
	else
		AddOutputPin("Out", MaterialPinType::Float4);
}

std::string ComponentMaskNode::GenerateCode(CompilerContext& ctx)
{
	std::string expr = ctx.GetInputExpression(m_ID, "In");
	std::string varName = ctx.AllocateVariable(m_ID, "mask");
	std::string mask = Properties["Mask"];

	// 将 RGBA 掩码转换为 GLSL swizzle 操作
	std::string swizzle;
	for (char c : mask)
	{
		switch (c)
		{
			case 'R': case 'r': swizzle += ".r"; break;
			case 'G': case 'g': swizzle += ".g"; break;
			case 'B': case 'b': swizzle += ".b"; break;
			case 'A': case 'a': swizzle += ".a"; break;
			default: swizzle += c; break;
		}
	}

	if (swizzle.empty())
		swizzle = ".rgb";

	auto* outPin = GetOutputPin("Out");
	std::string type = ToGLSLType(outPin->Type);

	ctx.EmitLine(type + " " + varName + " = " + expr + swizzle + ";");
	return varName;
}

// 向量节点自动注册
REGISTER_MATERIAL_NODE(DotProductNode, "DotProduct")
REGISTER_MATERIAL_NODE(CrossProductNode, "CrossProduct")
REGISTER_MATERIAL_NODE(NormalizeNode, "Normalize")
REGISTER_MATERIAL_NODE(AppendVectorNode, "AppendVector")
REGISTER_MATERIAL_NODE(ComponentMaskNode, "ComponentMask")

} // namespace AF
