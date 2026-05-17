#pragma once

// ============================================================================
// MaterialGraph.h — 材质图核心数据结构
//
// 功能：管理材质节点图的拓扑结构，提供节点添加/查询/连接/拓扑排序/环路检测等
//       操作，是整个材质系统的数据中枢。
// ============================================================================

#include "Core/Types.h"
#include "MaterialGraph/MaterialNode.h"

#include <cstdint>
#include <vector>

namespace AF {

// ============================================================================
// MaterialGraph — 材质节点图
//
// 维护一组 MaterialNode 构成的有向无环图(DAG)，节点间通过 Pin 连接。
// 支持拓扑排序(用于代码生成顺序)、环路检测、无效节点清理。
// ============================================================================
class MaterialGraph
{
public:
	// 添加一个节点到图中，返回裸指针以便外部访问
	MaterialNode* AddNode(Unique<MaterialNode> node);
	// 根据 NodeID 查找节点
	MaterialNode* GetNode(NodeID id) const;
	// 查找类型名为 "MaterialOutput" 的输出节点（图中应有且仅有一个）
	MaterialNode* FindOutputNode() const;
	// 移除所有不可达的孤立节点（从输出节点反向遍历可达性）
	void RemoveDeadNodes();

	// 在两个节点的指定引脚之间建立连接，自动进行类型兼容性检查
	bool Connect(NodeID srcNode, const std::string& srcPin,
	             NodeID dstNode, const std::string& dstPin);
	// 断开指定节点的某个输入引脚
	bool DisconnectInput(NodeID node, const std::string& pin);

	// 对图进行拓扑排序（Kahn 算法），返回节点执行顺序
	std::vector<MaterialNode*> TopologicalSort() const;
	// 验证图的合法性：无环路、有输出节点、所有连接类型兼容
	bool Validate() const;
	// 检测图中是否存在环路（DFS 三色标记法）
	bool HasCycles() const;
	// 分配一个自增的唯一节点 ID
	NodeID AllocateNodeID();

	// 访问节点列表
	const auto& GetNodes() const { return m_Nodes; }
	auto& GetNodes() { return m_Nodes; }
	// 返回节点数量
	size_t NodeCount() const { return m_Nodes.size(); }

private:
	std::vector<Unique<MaterialNode>> m_Nodes;  // 图中所有节点的唯一所有权
	uint32_t m_NextNodeID = 1;                   // 下一个可用的节点 ID（从 1 开始）
};

} // namespace AF
