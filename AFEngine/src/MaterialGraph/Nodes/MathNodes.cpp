#include "MaterialGraph/Nodes/MathNodes.h"
#include "MaterialGraph/NodeFactory.h"
#include "MaterialGraph/MaterialCompiler.h"

namespace AF {

// ============================================================================
// AddNode — A + B
// ============================================================================

AddNode::AddNode(NodeID id)
	: MaterialNode(id, "Add")
{
	AddInputPin("A", MaterialPinType::Float4, 0.0f);
	AddInputPin("B", MaterialPinType::Float4, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string AddNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "add");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = " + exprA + " + " + exprB + ";");
	return varName;
}

// ============================================================================
// SubtractNode — A - B
// ============================================================================

SubtractNode::SubtractNode(NodeID id)
	: MaterialNode(id, "Subtract")
{
	AddInputPin("A", MaterialPinType::Float4, 0.0f);
	AddInputPin("B", MaterialPinType::Float4, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string SubtractNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "sub");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = " + exprA + " - " + exprB + ";");
	return varName;
}

// ============================================================================
// MultiplyNode — A * B
// ============================================================================

MultiplyNode::MultiplyNode(NodeID id)
	: MaterialNode(id, "Multiply")
{
	AddInputPin("A", MaterialPinType::Float4, 0.0f);
	AddInputPin("B", MaterialPinType::Float4, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string MultiplyNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "mul");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = " + exprA + " * " + exprB + ";");
	return varName;
}

// ============================================================================
// DivideNode — A / B
// ============================================================================

DivideNode::DivideNode(NodeID id)
	: MaterialNode(id, "Divide")
{
	AddInputPin("A", MaterialPinType::Float4, 0.0f);
	AddInputPin("B", MaterialPinType::Float4, 1.0f);  // B 的默认值为 1（避免除以零）
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string DivideNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "div");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = " + exprA + " / " + exprB + ";");
	return varName;
}

// ============================================================================
// PowerNode — pow(Base, Exp)
// ============================================================================

PowerNode::PowerNode(NodeID id)
	: MaterialNode(id, "Power")
{
	AddInputPin("Base", MaterialPinType::Float, 1.0f);
	AddInputPin("Exp", MaterialPinType::Float, 1.0f);
	AddOutputPin("Out", MaterialPinType::Float);
}

std::string PowerNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprBase = ctx.GetInputExpression(m_ID, "Base");
	std::string exprExp = ctx.GetInputExpression(m_ID, "Exp");
	std::string varName = ctx.AllocateVariable(m_ID, "pow");

	ctx.EmitLine("float " + varName + " = pow(" + exprBase + ", " + exprExp + ");");
	return varName;
}

// ============================================================================
// SqrtNode — sqrt(In)
// ============================================================================

SqrtNode::SqrtNode(NodeID id)
	: MaterialNode(id, "Sqrt")
{
	AddInputPin("In", MaterialPinType::Float, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float);
}

std::string SqrtNode::GenerateCode(CompilerContext& ctx)
{
	std::string expr = ctx.GetInputExpression(m_ID, "In");
	std::string varName = ctx.AllocateVariable(m_ID, "sqrt");

	ctx.EmitLine("float " + varName + " = sqrt(" + expr + ");");
	return varName;
}

// ============================================================================
// ClampNode — clamp(V, Min, Max)
// ============================================================================

ClampNode::ClampNode(NodeID id)
	: MaterialNode(id, "Clamp")
{
	AddInputPin("V", MaterialPinType::Float4, 0.0f);
	AddInputPin("Min", MaterialPinType::Float4, 0.0f);
	AddInputPin("Max", MaterialPinType::Float4, 1.0f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string ClampNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprV = ctx.GetInputExpression(m_ID, "V");
	std::string exprMin = ctx.GetInputExpression(m_ID, "Min");
	std::string exprMax = ctx.GetInputExpression(m_ID, "Max");
	std::string varName = ctx.AllocateVariable(m_ID, "clamp");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = clamp(" + exprV + ", " + exprMin + ", " + exprMax + ");");
	return varName;
}

// ============================================================================
// SaturateNode — clamp(In, 0.0, 1.0)
// ============================================================================

SaturateNode::SaturateNode(NodeID id)
	: MaterialNode(id, "Saturate")
{
	AddInputPin("In", MaterialPinType::Float4, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string SaturateNode::GenerateCode(CompilerContext& ctx)
{
	std::string expr = ctx.GetInputExpression(m_ID, "In");
	std::string varName = ctx.AllocateVariable(m_ID, "saturate");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	// saturate 等价于 clamp to [0, 1]
	ctx.EmitLine(type + " " + varName + " = clamp(" + expr + ", 0.0, 1.0);");
	return varName;
}

// ============================================================================
// MinNode — min(A, B)
// ============================================================================

MinNode::MinNode(NodeID id)
	: MaterialNode(id, "Min")
{
	AddInputPin("A", MaterialPinType::Float4, 0.0f);
	AddInputPin("B", MaterialPinType::Float4, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string MinNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "min");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = min(" + exprA + ", " + exprB + ");");
	return varName;
}

// ============================================================================
// MaxNode — max(A, B)
// ============================================================================

MaxNode::MaxNode(NodeID id)
	: MaterialNode(id, "Max")
{
	AddInputPin("A", MaterialPinType::Float4, 0.0f);
	AddInputPin("B", MaterialPinType::Float4, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string MaxNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string varName = ctx.AllocateVariable(m_ID, "max");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = max(" + exprA + ", " + exprB + ");");
	return varName;
}

// ============================================================================
// AbsNode — abs(In)
// ============================================================================

AbsNode::AbsNode(NodeID id)
	: MaterialNode(id, "Abs")
{
	AddInputPin("In", MaterialPinType::Float4, 0.0f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string AbsNode::GenerateCode(CompilerContext& ctx)
{
	std::string expr = ctx.GetInputExpression(m_ID, "In");
	std::string varName = ctx.AllocateVariable(m_ID, "abs");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	ctx.EmitLine(type + " " + varName + " = abs(" + expr + ");");
	return varName;
}

// ============================================================================
// LerpNode — mix(A, B, Alpha)
// ============================================================================

LerpNode::LerpNode(NodeID id)
	: MaterialNode(id, "Lerp")
{
	AddInputPin("A", MaterialPinType::Float4, 0.0f);
	AddInputPin("B", MaterialPinType::Float4, 1.0f);
	AddInputPin("Alpha", MaterialPinType::Float, 0.5f);
	AddOutputPin("Out", MaterialPinType::Float4);
}

std::string LerpNode::GenerateCode(CompilerContext& ctx)
{
	std::string exprA = ctx.GetInputExpression(m_ID, "A");
	std::string exprB = ctx.GetInputExpression(m_ID, "B");
	std::string exprAlpha = ctx.GetInputExpression(m_ID, "Alpha");
	std::string varName = ctx.AllocateVariable(m_ID, "lerp");
	std::string type = ToGLSLType(GetOutputPin("Out")->Type);

	// GLSL 中使用 mix() 函数实现线性插值
	ctx.EmitLine(type + " " + varName + " = mix(" + exprA + ", " + exprB + ", " + exprAlpha + ");");
	return varName;
}

// 数学节点自动注册
REGISTER_MATERIAL_NODE(AddNode, "Add")
REGISTER_MATERIAL_NODE(SubtractNode, "Subtract")
REGISTER_MATERIAL_NODE(MultiplyNode, "Multiply")
REGISTER_MATERIAL_NODE(DivideNode, "Divide")
REGISTER_MATERIAL_NODE(PowerNode, "Power")
REGISTER_MATERIAL_NODE(SqrtNode, "Sqrt")
REGISTER_MATERIAL_NODE(ClampNode, "Clamp")
REGISTER_MATERIAL_NODE(SaturateNode, "Saturate")
REGISTER_MATERIAL_NODE(MinNode, "Min")
REGISTER_MATERIAL_NODE(MaxNode, "Max")
REGISTER_MATERIAL_NODE(AbsNode, "Abs")
REGISTER_MATERIAL_NODE(LerpNode, "Lerp")

} // namespace AF
