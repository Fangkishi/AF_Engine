#include "MaterialGraph/MaterialNode.h"

namespace AF {

// 构造函数：初始化节点 ID 和类型名
MaterialNode::MaterialNode(NodeID id, const std::string& typeName)
	: m_ID(id)
	, m_TypeName(typeName)
{
}

// 添加输入引脚：设置名称、类型和默认值，追加到 m_InputPins
void MaterialNode::AddInputPin(const std::string& name, MaterialPinType type, MaterialParamValue defaultValue)
{
	MaterialPin pin;
	pin.Name = name;
	pin.Type = type;
	pin.DefaultValue = defaultValue;
	m_InputPins.push_back(std::move(pin));
}

// 添加输出引脚：设置名称和类型，追加到 m_OutputPins
void MaterialNode::AddOutputPin(const std::string& name, MaterialPinType type)
{
	MaterialPin pin;
	pin.Name = name;
	pin.Type = type;
	m_OutputPins.push_back(std::move(pin));
}

// 根据名称查找输入引脚，返回 nullptr 表示未找到
MaterialPin* MaterialNode::GetInputPin(const std::string& name)
{
	for (auto& pin : m_InputPins)
	{
		if (pin.Name == name)
			return &pin;
	}
	return nullptr;
}

// 根据名称查找输出引脚，返回 nullptr 表示未找到
MaterialPin* MaterialNode::GetOutputPin(const std::string& name)
{
	for (auto& pin : m_OutputPins)
	{
		if (pin.Name == name)
			return &pin;
	}
	return nullptr;
}

} // namespace AF
