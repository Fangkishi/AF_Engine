#include "MaterialGraph/Nodes/ConstantNodes.h"
#include "MaterialGraph/NodeFactory.h"
#include "MaterialGraph/MaterialCompiler.h"

namespace AF {

// ============================================================================
// ScalarConstantNode
// ============================================================================

ScalarConstantNode::ScalarConstantNode(NodeID id, float defaultVal, bool exposeAsParam)
	: MaterialNode(id, "ScalarConstant")
{
	AddOutputPin("Out", MaterialPinType::Float);
	auto* pin = GetOutputPin("Out");
	if (pin)
		pin->DefaultValue = defaultVal;

	// 记录是否暴露为材质参数（可从材质编辑器调整）
	Properties["ExposeAsParameter"] = exposeAsParam ? "true" : "false";
}

// 生成代码：若 exposeAsParameter == true，从 materialParams UBO 取值；否则编译为 const
std::string ScalarConstantNode::GenerateCode(CompilerContext& ctx)
{
	auto* outPin = GetOutputPin("Out");
	std::string varName = ctx.AllocateVariable(m_ID, "const");

	if (Properties["ExposeAsParameter"] == "true")
	{
		std::string paramName = "Param_" + std::to_string(m_ID.Value);
		ctx.CollectParameter(paramName, outPin->Type, outPin->DefaultValue);
		ctx.EmitLine("float " + varName + " = materialParams." + paramName + ";");
	}
	else
	{
		std::string literal = ctx.GetLiteralExpression(outPin->DefaultValue);
		ctx.EmitLine("const float " + varName + " = " + literal + ";");
	}

	return varName;
}

// ============================================================================
// Vector2ConstantNode
// ============================================================================

Vector2ConstantNode::Vector2ConstantNode(NodeID id, const glm::vec2& defaultVal, bool exposeAsParam)
	: MaterialNode(id, "Vector2Constant")
{
	AddOutputPin("Out", MaterialPinType::Float2);
	auto* pin = GetOutputPin("Out");
	if (pin)
		pin->DefaultValue = defaultVal;

	Properties["ExposeAsParameter"] = exposeAsParam ? "true" : "false";
}

std::string Vector2ConstantNode::GenerateCode(CompilerContext& ctx)
{
	auto* outPin = GetOutputPin("Out");
	std::string varName = ctx.AllocateVariable(m_ID, "const");

	if (Properties["ExposeAsParameter"] == "true")
	{
		std::string paramName = "Param_" + std::to_string(m_ID.Value);
		ctx.CollectParameter(paramName, outPin->Type, outPin->DefaultValue);
		ctx.EmitLine("vec2 " + varName + " = materialParams." + paramName + ";");
	}
	else
	{
		std::string literal = ctx.GetLiteralExpression(outPin->DefaultValue);
		ctx.EmitLine("const vec2 " + varName + " = " + literal + ";");
	}

	return varName;
}

// ============================================================================
// Vector3ConstantNode
// ============================================================================

Vector3ConstantNode::Vector3ConstantNode(NodeID id, const glm::vec3& defaultVal, bool exposeAsParam)
	: MaterialNode(id, "Vector3Constant")
{
	AddOutputPin("Out", MaterialPinType::Float3);
	auto* pin = GetOutputPin("Out");
	if (pin)
		pin->DefaultValue = defaultVal;

	Properties["ExposeAsParameter"] = exposeAsParam ? "true" : "false";
}

std::string Vector3ConstantNode::GenerateCode(CompilerContext& ctx)
{
	auto* outPin = GetOutputPin("Out");
	std::string varName = ctx.AllocateVariable(m_ID, "const");

	if (Properties["ExposeAsParameter"] == "true")
	{
		std::string paramName = "Param_" + std::to_string(m_ID.Value);
		ctx.CollectParameter(paramName, outPin->Type, outPin->DefaultValue);
		ctx.EmitLine("vec3 " + varName + " = materialParams." + paramName + ";");
	}
	else
	{
		std::string literal = ctx.GetLiteralExpression(outPin->DefaultValue);
		ctx.EmitLine("const vec3 " + varName + " = " + literal + ";");
	}

	return varName;
}

// ============================================================================
// Vector4ConstantNode
// ============================================================================

Vector4ConstantNode::Vector4ConstantNode(NodeID id, const glm::vec4& defaultVal, bool exposeAsParam)
	: MaterialNode(id, "Vector4Constant")
{
	AddOutputPin("Out", MaterialPinType::Float4);
	auto* pin = GetOutputPin("Out");
	if (pin)
		pin->DefaultValue = defaultVal;

	Properties["ExposeAsParameter"] = exposeAsParam ? "true" : "false";
}

std::string Vector4ConstantNode::GenerateCode(CompilerContext& ctx)
{
	auto* outPin = GetOutputPin("Out");
	std::string varName = ctx.AllocateVariable(m_ID, "const");

	if (Properties["ExposeAsParameter"] == "true")
	{
		std::string paramName = "Param_" + std::to_string(m_ID.Value);
		ctx.CollectParameter(paramName, outPin->Type, outPin->DefaultValue);
		ctx.EmitLine("vec4 " + varName + " = materialParams." + paramName + ";");
	}
	else
	{
		std::string literal = ctx.GetLiteralExpression(outPin->DefaultValue);
		ctx.EmitLine("const vec4 " + varName + " = " + literal + ";");
	}

	return varName;
}

// ============================================================================
// ColorConstantNode
// ============================================================================

ColorConstantNode::ColorConstantNode(NodeID id, const glm::vec3& defaultVal, bool exposeAsParam)
	: MaterialNode(id, "ColorConstant")
{
	AddOutputPin("Out", MaterialPinType::Color3);
	auto* pin = GetOutputPin("Out");
	if (pin)
		pin->DefaultValue = defaultVal;

	Properties["ExposeAsParameter"] = exposeAsParam ? "true" : "false";
}

std::string ColorConstantNode::GenerateCode(CompilerContext& ctx)
{
	auto* outPin = GetOutputPin("Out");
	std::string varName = ctx.AllocateVariable(m_ID, "color");

	if (Properties["ExposeAsParameter"] == "true")
	{
		std::string paramName = "Param_" + std::to_string(m_ID.Value);
		ctx.CollectParameter(paramName, outPin->Type, outPin->DefaultValue);
		ctx.EmitLine("vec3 " + varName + " = materialParams." + paramName + ";");
	}
	else
	{
		std::string literal = ctx.GetLiteralExpression(outPin->DefaultValue);
		ctx.EmitLine("const vec3 " + varName + " = " + literal + ";");
	}

	return varName;
}

// 常量节点自动注册
REGISTER_MATERIAL_NODE(ScalarConstantNode, "ScalarConstant")
REGISTER_MATERIAL_NODE(Vector2ConstantNode, "Vector2Constant")
REGISTER_MATERIAL_NODE(Vector3ConstantNode, "Vector3Constant")
REGISTER_MATERIAL_NODE(Vector4ConstantNode, "Vector4Constant")
REGISTER_MATERIAL_NODE(ColorConstantNode, "ColorConstant")

} // namespace AF
