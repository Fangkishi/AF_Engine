#pragma once

// ============================================================================
// NodeFactory.h — 材质节点工厂
//
// 功能：基于类型名称的动态节点创建工厂。节点类通过 REGISTER_MATERIAL_NODE
//       宏自动注册到工厂中，支持运行时按名称创建任意已注册的节点类型。
// ============================================================================

#include "Core/Types.h"
#include "MaterialGraph/MaterialNode.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace AF {

// ============================================================================
// NodeFactory — 节点工厂（单例注册表）
//
// 维护一个从类型名到创建函数的映射表。所有具体的材质节点类型在 .cpp 文件
// 中通过 REGISTER_MATERIAL_NODE 宏完成自动注册。
// ============================================================================
class NodeFactory
{
public:
	// 创建函数类型：接收 NodeID，返回 Unique<MaterialNode>
	using Creator = std::function<Unique<MaterialNode>(NodeID)>;

	// 注册一个节点类型
	static void Register(const std::string& typeName, Creator creator);
	// 根据类型名创建节点实例
	static Unique<MaterialNode> Create(const std::string& typeName, NodeID id);
	// 获取所有已注册类型的名称列表
	static std::vector<std::string> GetAllTypes();

private:
	// 获取内部静态注册表
	static std::unordered_map<std::string, Creator>& GetRegistry();
};

} // namespace AF

// 节点自动注册宏：定义静态变量，在程序启动时自动将节点类型注册到工厂
// 用法：REGISTER_MATERIAL_NODE(AddNode, "Add")
#define REGISTER_MATERIAL_NODE(ClassName, Name) \
	static bool _reg_##ClassName = []() { \
		AF::NodeFactory::Register(Name, [](AF::NodeID id) { return std::make_unique<ClassName>(id); }); \
		return true; \
	}();
