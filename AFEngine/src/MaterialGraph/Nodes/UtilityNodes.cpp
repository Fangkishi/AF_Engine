#include "MaterialGraph/Nodes/UtilityNodes.h"
#include "MaterialGraph/NodeFactory.h"
#include "MaterialGraph/MaterialCompiler.h"

namespace AF {

// ============================================================================
// FresnelNode — 菲涅尔效果
//
// 公式：pow(1.0 - abs(dot(Normal, ViewDirection)), Exponent)
// 法线朝向视线时值最小（中心），法线垂直视线时值最大（边缘）。
// ============================================================================

FresnelNode::FresnelNode(NodeID id)
	: MaterialNode(id, "Fresnel")
{
	AddInputPin("Normal", MaterialPinType::Float3, glm::vec3(0.0f, 0.0f, 1.0f));
	AddInputPin("Exponent", MaterialPinType::Float, 5.0f);
	AddOutputPin("Out", MaterialPinType::Float);
}

std::string FresnelNode::GenerateCode(CompilerContext& ctx)
{
	std::string normalExpr = ctx.GetInputExpression(m_ID, "Normal");
	std::string exponentExpr = ctx.GetInputExpression(m_ID, "Exponent");
	std::string varName = ctx.AllocateVariable(m_ID, "fresnel");

	ctx.EmitLine(
		"float " + varName + " = pow(1.0 - abs(dot(" + normalExpr + ", frameData.ViewDirection)), " + exponentExpr + ");"
	);
	return varName;
}

// ============================================================================
// TimeNode — 时间节点，输出 frameData.Time（秒）
// ============================================================================

TimeNode::TimeNode(NodeID id)
	: MaterialNode(id, "Time")
{
	AddOutputPin("Out", MaterialPinType::Float);
}

std::string TimeNode::GenerateCode(CompilerContext& ctx)
{
	std::string varName = ctx.AllocateVariable(m_ID, "time");

	ctx.EmitLine("float " + varName + " = frameData.Time;");
	return varName;
}

// ============================================================================
// TexCoordNode — 纹理坐标节点
// 根据 Index（0 或 1）选择 UV0 或 UV1 坐标集
// ============================================================================

TexCoordNode::TexCoordNode(NodeID id)
	: MaterialNode(id, "TexCoord")
{
	AddInputPin("Index", MaterialPinType::Float, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float2);
}

std::string TexCoordNode::GenerateCode(CompilerContext& ctx)
{
	std::string indexExpr = ctx.GetInputExpression(m_ID, "Index");
	std::string varName = ctx.AllocateVariable(m_ID, "uv");

	ctx.EmitLine(
		"vec2 " + varName + " = int(" + indexExpr + ") == 1 ? materialParams.UV1 : materialParams.UV0;"
	);
	return varName;
}

// ============================================================================
// PannerNode — UV 平移
// UVs + Speed * Time，产生纹理滚动效果
// ============================================================================

PannerNode::PannerNode(NodeID id)
	: MaterialNode(id, "Panner")
{
	AddInputPin("UVs", MaterialPinType::Float2, glm::vec2(0.0f));
	AddInputPin("Speed", MaterialPinType::Float2, glm::vec2(0.0f));
	AddOutputPin("Out", MaterialPinType::Float2);
}

std::string PannerNode::GenerateCode(CompilerContext& ctx)
{
	std::string uvExpr = ctx.GetInputExpression(m_ID, "UVs");
	std::string speedExpr = ctx.GetInputExpression(m_ID, "Speed");
	std::string varName = ctx.AllocateVariable(m_ID, "panner");

	ctx.EmitLine(
		"vec2 " + varName + " = " + uvExpr + " + " + speedExpr + " * frameData.Time;"
	);
	return varName;
}

// ============================================================================
// RotatorNode — UV 旋转
// 将 UV 平移到中心点，绕中心旋转后再平移回原位置
// ============================================================================

RotatorNode::RotatorNode(NodeID id)
	: MaterialNode(id, "Rotator")
{
	AddInputPin("UVs", MaterialPinType::Float2, glm::vec2(0.0f));
	AddInputPin("Center", MaterialPinType::Float2, glm::vec2(0.5f));
	AddInputPin("Angle", MaterialPinType::Float, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float2);
}

std::string RotatorNode::GenerateCode(CompilerContext& ctx)
{
	std::string uvExpr = ctx.GetInputExpression(m_ID, "UVs");
	std::string centerExpr = ctx.GetInputExpression(m_ID, "Center");
	std::string angleExpr = ctx.GetInputExpression(m_ID, "Angle");
	std::string varName = ctx.AllocateVariable(m_ID, "rotator");

	// 平移 UV 使中心点位于原点
	ctx.EmitLine(
		"vec2 " + varName + " = " + uvExpr + " - " + centerExpr + ";"
	);
	// 旋转矩阵：x' = x*cos - y*sin, y' = x*sin + y*cos，再加回中心偏移
	ctx.EmitLine(
		varName + " = vec2(" +
		varName + ".x * cos(" + angleExpr + ") - " + varName + ".y * sin(" + angleExpr + "), " +
		varName + ".x * sin(" + angleExpr + ") + " + varName + ".y * cos(" + angleExpr + ")) + " + centerExpr + ";"
	);
	return varName;
}

// 工具节点自动注册
REGISTER_MATERIAL_NODE(FresnelNode, "Fresnel")
REGISTER_MATERIAL_NODE(TimeNode, "Time")
REGISTER_MATERIAL_NODE(TexCoordNode, "TexCoord")
REGISTER_MATERIAL_NODE(PannerNode, "Panner")
REGISTER_MATERIAL_NODE(RotatorNode, "Rotator")

} // namespace AF
