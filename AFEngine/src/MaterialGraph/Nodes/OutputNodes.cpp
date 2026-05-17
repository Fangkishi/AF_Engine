#include "MaterialGraph/Nodes/OutputNodes.h"
#include "MaterialGraph/NodeFactory.h"
#include "MaterialGraph/MaterialCompiler.h"

namespace AF {

// ============================================================================
// MaterialOutputNode — 材质输出节点
//
// 每个材质图有且仅有一个 MaterialOutputNode，作为材质编译的终点。
// 该节点为每个材质属性生成一个材质属性获取函数（如 Material_GetBaseColor），
// 供主着色器在计算光照时调用。
// ============================================================================

MaterialOutputNode::MaterialOutputNode(NodeID id)
	: MaterialNode(id, "MaterialOutput")
{
	// 定义所有材质输出属性引脚，每个引脚类型为 MaterialAttr
	AddInputPin("BaseColor", MaterialPinType::MaterialAttr, glm::vec3(0.5f, 0.5f, 0.5f));
	AddInputPin("Metallic", MaterialPinType::MaterialAttr, 0.0f);
	AddInputPin("Roughness", MaterialPinType::MaterialAttr, 0.5f);
	AddInputPin("AO", MaterialPinType::MaterialAttr, 1.0f);
	AddInputPin("NormalWS", MaterialPinType::MaterialAttr, glm::vec3(0.0f, 0.0f, 1.0f));
	AddInputPin("Emissive", MaterialPinType::MaterialAttr, glm::vec3(0.0f));
	AddInputPin("OpacityMask", MaterialPinType::MaterialAttr, 1.0f);
	AddInputPin("Opacity", MaterialPinType::MaterialAttr, 1.0f);
}

// 为每个输入引脚生成一个材质属性获取函数
// 如 Material_GetBaseColor(MaterialParams params) { return ...; }
// 如果引脚已连接，返回上游生成的表达式；否则返回默认值字面量
std::string MaterialOutputNode::GenerateCode(CompilerContext& ctx)
{
	for (auto& pin : m_InputPins)
	{
		std::string funcName = "Material_Get" + pin.Name;

		// 从 DefaultValue 推断实际数据类型（pin.Type 是 MaterialAttr，无法直接映射 GLSL）
		MaterialPinType dataType = MaterialPinType::Float;
		if (std::holds_alternative<glm::vec2>(pin.DefaultValue))      dataType = MaterialPinType::Float2;
		else if (std::holds_alternative<glm::vec3>(pin.DefaultValue)) dataType = MaterialPinType::Float3;
		else if (std::holds_alternative<glm::vec4>(pin.DefaultValue)) dataType = MaterialPinType::Float4;
		else if (std::holds_alternative<float>(pin.DefaultValue))     dataType = MaterialPinType::Float;
		else if (std::holds_alternative<int32_t>(pin.DefaultValue))   dataType = MaterialPinType::Float;
		else if (std::holds_alternative<bool>(pin.DefaultValue))      dataType = MaterialPinType::Boolean;

		std::string retType = ToGLSLType(dataType);
		std::string bodyExpr;

		if (pin.IsConnected())
		{
			bodyExpr = ctx.GetInputExpression(m_ID, pin.Name);
		}
		else
		{
			bodyExpr = "params." + pin.Name;
		}

		// 将 pin 声明为 UBO 参数，消除下游硬编码
		ctx.CollectParameter(pin.Name, dataType, pin.DefaultValue);

		ctx.EmitFunction(retType, funcName, "MaterialParams params", "    return " + bodyExpr + ";");
	}

	return "";
}

REGISTER_MATERIAL_NODE(MaterialOutputNode, "MaterialOutput")

} // namespace AF
