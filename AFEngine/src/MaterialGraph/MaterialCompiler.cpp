#include "MaterialGraph/MaterialCompiler.h"
#include "MaterialGraph/MaterialGraph.h"
#include "MaterialGraph/MaterialNode.h"

#include <glm/glm.hpp>

namespace AF {

CompilerContext::CompilerContext(const MaterialGraph& graph)
	: m_Graph(graph)
{
}

// 发射一行 GLSL 代码，追加换行符
void CompilerContext::EmitLine(const std::string& line)
{
	m_Code << line << "\n";
}

// 发射一个 GLSL 函数定义：返回值类型、函数名、参数列表和函数体
void CompilerContext::EmitFunction(const std::string& retType, const std::string& name,
                                    const std::string& params, const std::string& body)
{
	m_Code << retType << " " << name << "(" << params << ")\n{\n" << body << "\n}\n\n";
}

// 为节点分配一个临时变量名，格式为 "_n{NodeID}_{hint}"
// 会自动将 hNit 与输出引脚关联（按引脚顺序从左到右匹配）
std::string CompilerContext::AllocateVariable(NodeID node, const std::string& hint)
{
	std::string varName = "_n" + std::to_string(node.Value) + "_" + hint;

	auto& nodeOutput = m_NodeOutputs[node.Value];

	// 如果该节点还没为输出引脚分配变量，将新变量和输出引脚按序关联
	const MaterialNode* graphNode = m_Graph.GetNode(node);
	if (graphNode)
	{
		const auto& outputPins = graphNode->GetOutputPins();
		size_t assignedCount = nodeOutput.PinToVar.size();
		if (assignedCount < outputPins.size())
		{
			nodeOutput.PinToVar[outputPins[assignedCount].Name] = varName;
		}
	}

	// 同时也用 hint 做一次记录
	nodeOutput.PinToVar[hint] = varName;

	return varName;
}

// 获取输入引脚的表达式：
// - 如果引脚已连接，返回上游节点对应输出变量的名称（必要时插入类型转换）
// - 如果未连接，返回该引脚默认值的字面量
std::string CompilerContext::GetInputExpression(NodeID node, const std::string& pinName)
{
	MaterialNode* graphNode = m_Graph.GetNode(node);
	if (!graphNode) return "";

	MaterialPin* pin = graphNode->GetInputPin(pinName);
	if (!pin) return "";

	// 未连接：返回默认值的字面量
	if (!pin->IsConnected())
		return GetLiteralExpression(pin->DefaultValue);

	// 获取上游节点
	MaterialNode* upstreamNode = m_Graph.GetNode(pin->ConnectedNode);
	if (!upstreamNode) return GetLiteralExpression(pin->DefaultValue);

	const auto& upstreamOutputs = upstreamNode->GetOutputPins();
	if (pin->ConnectedPin.Value >= upstreamOutputs.size())
		return GetLiteralExpression(pin->DefaultValue);

	std::string upstreamPinName = upstreamOutputs[pin->ConnectedPin.Value].Name;

	// 在 m_NodeOutputs 中查找上游节点输出变量的名称
	auto it = m_NodeOutputs.find(pin->ConnectedNode.Value);
	if (it != m_NodeOutputs.end())
	{
		auto pit = it->second.PinToVar.find(upstreamPinName);
		if (pit != it->second.PinToVar.end())
		{
			std::string expr = pit->second;

			// 如果源和目标类型不同，插入类型转换表达式
			const MaterialPin& upstreamPin = upstreamOutputs[pin->ConnectedPin.Value];
			if (pin->Type != upstreamPin.Type)
			{
				if (AreTypesCompatible(upstreamPin.Type, pin->Type))
				{
					expr = CastExpression(expr, upstreamPin.Type, pin->Type);
				}
			}
			return expr;
		}
	}

	return GetLiteralExpression(pin->DefaultValue);
}

// 获取输出引脚的表达式（即之前分配的变量名）
std::string CompilerContext::GetOutputExpression(NodeID node, const std::string& pinName)
{
	auto it = m_NodeOutputs.find(node.Value);
	if (it != m_NodeOutputs.end())
	{
		auto pit = it->second.PinToVar.find(pinName);
		if (pit != it->second.PinToVar.end())
			return pit->second;
	}
	return "";
}

// 将 MaterialParamValue 转为 GLSL 字面量字符串
// 支持：float, vec2, vec3, vec4, int, bool
std::string CompilerContext::GetLiteralExpression(const MaterialParamValue& val)
{
	if (std::holds_alternative<float>(val))
		return std::to_string(std::get<float>(val));
	if (std::holds_alternative<glm::vec2>(val))
	{
		auto& v = std::get<glm::vec2>(val);
		return "vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
	}
	if (std::holds_alternative<glm::vec3>(val))
	{
		auto& v = std::get<glm::vec3>(val);
		return "vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
	}
	if (std::holds_alternative<glm::vec4>(val))
	{
		auto& v = std::get<glm::vec4>(val);
		return "vec4(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ", " + std::to_string(v.w) + ")";
	}
	if (std::holds_alternative<int32_t>(val))
		return std::to_string(std::get<int32_t>(val));
	if (std::holds_alternative<bool>(val))
		return std::get<bool>(val) ? "true" : "false";
	return "vec3(0.0)";
}

// 生成 GLSL 类型转换表达式：
// float → vec2/vec3/vec4 通过构造函数补零实现
// vec2 → vec3/vec4、vec3 → vec4 类似
std::string CompilerContext::CastExpression(const std::string& expr,
                                             MaterialPinType from, MaterialPinType to)
{
	if (from == to) return expr;

	if (from == MaterialPinType::Float && to == MaterialPinType::Float2)
		return "vec2(" + expr + ", 0.0)";
	if (from == MaterialPinType::Float && to == MaterialPinType::Float3)
		return "vec3(" + expr + ", 0.0, 0.0)";
	if (from == MaterialPinType::Float && to == MaterialPinType::Float4)
		return "vec4(" + expr + ", 0.0, 0.0, 0.0)";
	if (from == MaterialPinType::Float2 && to == MaterialPinType::Float3)
		return "vec3(" + expr + ", 0.0)";
	if (from == MaterialPinType::Float2 && to == MaterialPinType::Float4)
		return "vec4(" + expr + ", 0.0, 0.0)";
	if (from == MaterialPinType::Float3 && to == MaterialPinType::Float4)
		return "vec4(" + expr + ", 0.0)";

	return expr;
}

// 收集材质参数（将加入 UBO 定义的材质参数列表）
void CompilerContext::CollectParameter(const std::string& name, MaterialPinType type,
                                        const MaterialParamValue& defaultValue)
{
	MaterialParameterDesc desc;
	desc.Name = name;
	desc.Type = type;
	desc.DefaultValue = defaultValue;
	m_CollectedParams.push_back(std::move(desc));
}

// 添加一条 GLSL 宏定义（用于变体条件编译）
void CompilerContext::AddDefine(const std::string& define)
{
	m_CollectedDefines.push_back(define);
}

// 结束编译，组装最终的 ShaderSnippet
// 包含文件头注释、GLSL 代码、参数列表和宏定义
ShaderSnippet CompilerContext::Finalize(const std::string& materialName)
{
	ShaderSnippet snippet;
	snippet.GLSLCode =
		"// @generated by MaterialCompiler\n"
		"// Material: " + materialName + "\n\n"
		+ m_Code.str();
	snippet.Parameters = std::move(m_CollectedParams);
	snippet.Defines = std::move(m_CollectedDefines);
	return snippet;
}

// ============================================================================
// MaterialCompiler 静态方法实现
// ============================================================================

// 编译入口：验证 → 拓扑排序 → 逐节点代码生成 → Finalize
ShaderSnippet MaterialCompiler::Compile(const MaterialGraph& graph, const std::string& materialName)
{
	if (!graph.Validate())
		return {};

	auto sorted = graph.TopologicalSort();

	CompilerContext ctx(graph);

	for (auto* node : sorted)
	{
		node->GenerateCode(ctx);
	}

	return ctx.Finalize(materialName);
}

bool MaterialCompiler::Validate(const MaterialGraph& graph)
{
	return graph.Validate();
}

std::vector<MaterialNode*> MaterialCompiler::GetSortedNodes(const MaterialGraph& graph)
{
	return graph.TopologicalSort();
}

void MaterialCompiler::GenerateCode(const std::vector<MaterialNode*>& sorted, CompilerContext& ctx)
{
	for (auto* node : sorted)
	{
		node->GenerateCode(ctx);
	}
}

} // namespace AF
