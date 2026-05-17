#include "MaterialGraph/NodeFactory.h"

namespace AF {

// 获取内部静态注册表（单例模式，函数局部静态变量）
std::unordered_map<std::string, NodeFactory::Creator>& NodeFactory::GetRegistry()
{
	static std::unordered_map<std::string, Creator> registry;
	return registry;
}

// 注册一个节点类型到工厂
void NodeFactory::Register(const std::string& typeName, Creator creator)
{
	GetRegistry()[typeName] = std::move(creator);
}

// 根据类型名创建节点实例，返回 nullptr 表示类型未注册
Unique<MaterialNode> NodeFactory::Create(const std::string& typeName, NodeID id)
{
	auto& registry = GetRegistry();
	auto it = registry.find(typeName);
	if (it != registry.end())
		return it->second(id);
	return nullptr;
}

// 获取所有已注册类型名称的列表
std::vector<std::string> NodeFactory::GetAllTypes()
{
	std::vector<std::string> types;
	for (auto& [name, _] : GetRegistry())
		types.push_back(name);
	return types;
}

} // namespace AF
