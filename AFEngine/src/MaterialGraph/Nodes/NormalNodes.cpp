#include "MaterialGraph/Nodes/NormalNodes.h"
#include "MaterialGraph/NodeFactory.h"
#include "MaterialGraph/MaterialCompiler.h"

namespace AF {

// ============================================================================
// NormalFromHeightmapNode — 从高度图生成法线贴图
//
// 使用有限差分法：采样相邻四个像素的高度值，计算坡度并构建法线。
// texelSize 控制采样步长，Strength 控制法线强度（值越小法线越明显）。
// ============================================================================

NormalFromHeightmapNode::NormalFromHeightmapNode(NodeID id)
	: MaterialNode(id, "NormalFromHeightmap")
{
	AddInputPin("Heightmap", MaterialPinType::Texture2D);
	AddInputPin("UVs", MaterialPinType::Float2, glm::vec2(0.0f));
	AddInputPin("Strength", MaterialPinType::Float, 1.0f);
	AddOutputPin("NormalWS", MaterialPinType::Float3);
}

std::string NormalFromHeightmapNode::GenerateCode(CompilerContext& ctx)
{
	std::string texExpr = ctx.GetInputExpression(m_ID, "Heightmap");
	std::string uvExpr = ctx.GetInputExpression(m_ID, "UVs");
	std::string strengthExpr = ctx.GetInputExpression(m_ID, "Strength");
	std::string varName = ctx.AllocateVariable(m_ID, "normal_ws");

	// 定义采样步长（硬编码 0.001，实际应用中可由属性控制）
	ctx.EmitLine("vec2 " + varName + "_texelSize = vec2(0.001, 0.0);");

	// 采样四个方向的高度值
	ctx.EmitLine("float " + varName + "_tl = texture(" + texExpr + ", " + uvExpr + " + " + varName + "_texelSize.xy).r;");
	ctx.EmitLine("float " + varName + "_tr = texture(" + texExpr + ", " + uvExpr + " - " + varName + "_texelSize.xy).r;");
	ctx.EmitLine("float " + varName + "_tu = texture(" + texExpr + ", " + uvExpr + " + " + varName + "_texelSize.yx).r;");
	ctx.EmitLine("float " + varName + "_td = texture(" + texExpr + ", " + uvExpr + " - " + varName + "_texelSize.yx).r;");

	// 由差分值构建法线：x = tl - tr, y = tu - td, z = 1/Strength，然后归一化
	ctx.EmitLine(
		"vec3 " + varName + " = normalize(vec3(" +
		varName + "_tl - " + varName + "_tr, " +
		varName + "_tu - " + varName + "_td, 1.0 / " + strengthExpr + "));"
	);

	return varName;
}

// ============================================================================
// BlendAngleCorrectedNormalsNode — 角度校正法线混合
//
// 使用 UDN（法线混合）算法将细节法线叠加到基础法线上。
// 比简单的 linear blend 能更好地保持法线细节方向。
// ============================================================================

BlendAngleCorrectedNormalsNode::BlendAngleCorrectedNormalsNode(NodeID id)
	: MaterialNode(id, "BlendAngleCorrectedNormals")
{
	AddInputPin("BaseNormal", MaterialPinType::Float3, glm::vec3(0.0f, 0.0f, 1.0f));
	AddInputPin("DetailNormal", MaterialPinType::Float3, glm::vec3(0.0f, 0.0f, 1.0f));
	AddOutputPin("Normal", MaterialPinType::Float3);
}

std::string BlendAngleCorrectedNormalsNode::GenerateCode(CompilerContext& ctx)
{
	std::string baseExpr = ctx.GetInputExpression(m_ID, "BaseNormal");
	std::string detailExpr = ctx.GetInputExpression(m_ID, "DetailNormal");
	std::string varName = ctx.AllocateVariable(m_ID, "blend_norm");

	// UDN 混合公式：
	// r1 = base.xyz + vec3(0, 0, 1)
	// r2 = detail（xy 取反）+ vec3(0, 0, 1)
	// result = normalize(r1 * dot(r1, r2) - r2 * r1.z)
	ctx.EmitLine(
		"vec3 " + varName + "_r1 = " + baseExpr + ".xyzx + vec3(0.0, 0.0, 1.0, 0.0);"
	);
	ctx.EmitLine(
		"vec3 " + varName + "_r2 = vec3(-" + detailExpr + ".xy, " + detailExpr + ".z) + vec3(0.0, 0.0, 1.0);"
	);
	ctx.EmitLine(
		"vec3 " + varName + " = normalize(" +
		varName + "_r1 * dot(" + varName + "_r1, " + varName + "_r2) - " +
		varName + "_r2 * " + varName + "_r1.z);"
	);

	return varName;
}

// 法线节点自动注册
REGISTER_MATERIAL_NODE(NormalFromHeightmapNode, "NormalFromHeightmap")
REGISTER_MATERIAL_NODE(BlendAngleCorrectedNormalsNode, "BlendAngleCorrectedNormals")

} // namespace AF
