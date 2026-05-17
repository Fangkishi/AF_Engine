#pragma once

// ============================================================================
// MaterialNode.h — 材质图节点基类
//
// 功能：定义材质图中所有节点的抽象基类。每个节点有唯一的 NodeID、类型名、
//       一组输入/输出引脚，以及一个纯虚的代码生成接口 GenerateCode()。
// ============================================================================

#include "Core/Types.h"
#include "MaterialGraph/MaterialPin.h"

#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace AF {

class CompilerContext;

// ============================================================================
// MaterialNode — 材质图节点基类
//
// 所有具体节点类型（如 AddNode, TextureSampleNode）都继承自此类。
// 子类必须实现 GenerateCode() 来生成对应的 GLSL 代码片段。
// 节点持有输入/输出引脚列表，以及编辑器中显示的位置信息。
// ============================================================================
class MaterialNode
{
public:
	MaterialNode(NodeID id, const std::string& typeName);
	virtual ~MaterialNode() = default;

	NodeID GetID() const { return m_ID; }
	const std::string& GetTypeName() const { return m_TypeName; }

	// 添加输入引脚（需指定名称、类型和可选的默认值）
	void AddInputPin(const std::string& name, MaterialPinType type, MaterialParamValue defaultValue = {});
	// 添加输出引脚（需指定名称和类型）
	void AddOutputPin(const std::string& name, MaterialPinType type);

	// 根据名称查找输入/输出引脚
	MaterialPin* GetInputPin(const std::string& name);
	MaterialPin* GetOutputPin(const std::string& name);
	// 获取输入/输出引脚列表
	const std::vector<MaterialPin>& GetInputPins() const { return m_InputPins; }
	const std::vector<MaterialPin>& GetOutputPins() const { return m_OutputPins; }

	// 获取该节点所需的变体宏定义（用于 ShaderVariant 系统），默认为空
	virtual std::vector<std::string> GetVariantDefines() const { return {}; }
	// 生成该节点对应的 GLSL 代码（纯虚，由子类实现）
	virtual std::string GenerateCode(CompilerContext& ctx) = 0;

	// 节点在编辑器中的位置（ImGui 坐标）
	glm::vec2 EditorPosition = { 0.0f, 0.0f };
	// 通用属性字典，存储各节点类型的特有配置（如 ExposeAsParameter, Mask 等）
	std::unordered_map<std::string, std::string> Properties;

protected:
	NodeID m_ID;                             // 节点唯一标识
	std::string m_TypeName;                  // 节点类型名称（用于工厂创建和序列化）
	std::vector<MaterialPin> m_InputPins;    // 输入引脚列表
	std::vector<MaterialPin> m_OutputPins;   // 输出引脚列表
};

// 材质节点工厂函数类型：接收 NodeID，返回唯一所有权的节点实例
using MaterialNodeCreator = std::function<Unique<MaterialNode>(NodeID)>;

} // namespace AF
