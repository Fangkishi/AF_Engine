#include "MaterialGraph/MaterialGraph.h"
#include "MaterialGraph/MaterialPin.h"

#include "Core/Log.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace AF {

// 添加节点：将节点移动到 m_Nodes 中，返回裸指针供后续连接操作
MaterialNode* MaterialGraph::AddNode(Unique<MaterialNode> node)
{
	MaterialNode* ptr = node.get();
	m_Nodes.push_back(std::move(node));
	return ptr;
}

// 根据 NodeID 线性查找节点
MaterialNode* MaterialGraph::GetNode(NodeID id) const
{
	for (auto& node : m_Nodes)
	{
		if (node->GetID().Value == id.Value)
			return node.get();
	}
	return nullptr;
}

// 从图中查找类型名为 "MaterialOutput" 的输出节点
// 每个材质图应有且仅有一个输出节点
MaterialNode* MaterialGraph::FindOutputNode() const
{
	for (auto& node : m_Nodes)
	{
		if (node->GetTypeName() == "MaterialOutput")
			return node.get();
	}
	return nullptr;
}

// 移除所有不可达节点（死节点）
// 从输出节点出发沿输入引脚反向遍历，标记所有可达节点，然后删除未标记的节点
void MaterialGraph::RemoveDeadNodes()
{
	MaterialNode* outputNode = FindOutputNode();
	if (!outputNode)
		return;

	// BFS 从输出节点出发，沿输入引脚反向遍历以收集所有可达节点
	std::unordered_set<NodeID> reachable;
	std::queue<MaterialNode*> queue;
	queue.push(outputNode);
	reachable.insert(outputNode->GetID());

	while (!queue.empty())
	{
		MaterialNode* current = queue.front();
		queue.pop();

		// 遍历当前节点的所有输入引脚，找到上游节点
		for (auto& inputPin : current->GetInputPins())
		{
			if (inputPin.IsConnected())
			{
				MaterialNode* upstream = GetNode(inputPin.ConnectedNode);
				if (upstream && reachable.find(upstream->GetID()) == reachable.end())
				{
					reachable.insert(upstream->GetID());
					queue.push(upstream);
				}
			}
		}
	}

	// 擦除不在 reachable 集合中的节点
	m_Nodes.erase(
		std::remove_if(m_Nodes.begin(), m_Nodes.end(),
			[&reachable](const Unique<MaterialNode>& node) {
				return reachable.find(node->GetID()) == reachable.end();
			}),
		m_Nodes.end()
	);
}

// 在两个节点之间建立引脚连接
// srcNode/srcPin: 源节点和输出引脚名
// dstNode/dstPin: 目标节点和输入引脚名
// 自动检查节点/引脚存在性和类型兼容性
bool MaterialGraph::Connect(NodeID srcNode, const std::string& srcPin,
                            NodeID dstNode, const std::string& dstPin)
{
	MaterialNode* src = GetNode(srcNode);
	MaterialNode* dst = GetNode(dstNode);

	if (!src || !dst)
	{
		AF_LOG_WARN("MaterialGraph::Connect: node not found");
		return false;
	}

	// 查找源节点的输出引脚
	MaterialPin* srcPinPtr = src->GetOutputPin(srcPin);
	if (!srcPinPtr)
	{
		AF_LOG_WARN("MaterialGraph::Connect: source pin '{}' not found on node '{}'",
		            srcPin, src->GetTypeName());
		return false;
	}

	// 查找目标节点的输入引脚
	MaterialPin* dstPinPtr = dst->GetInputPin(dstPin);
	if (!dstPinPtr)
	{
		AF_LOG_WARN("MaterialGraph::Connect: destination pin '{}' not found on node '{}'",
		            dstPin, dst->GetTypeName());
		return false;
	}

	// 检查引脚类型是否兼容
	if (!AreTypesCompatible(srcPinPtr->Type, dstPinPtr->Type))
	{
		AF_LOG_WARN("MaterialGraph::Connect: incompatible types ({} -> {})",
		            ToGLSLType(srcPinPtr->Type), ToGLSLType(dstPinPtr->Type));
		return false;
	}

	// 计算源输出引脚在输出列表中的索引，用于记录连接信息
	uint32_t srcPinIndex = 0;
	const auto& srcOutputs = src->GetOutputPins();
	for (size_t i = 0; i < srcOutputs.size(); ++i)
	{
		if (srcOutputs[i].Name == srcPin)
		{
			srcPinIndex = static_cast<uint32_t>(i);
			break;
		}
	}

	// 在目标输入引脚上记录连接信息
	dstPinPtr->ConnectedNode = srcNode;
	dstPinPtr->ConnectedPin = PinID{ srcPinIndex };

	return true;
}

// 断开指定节点的某个输入引脚连接，重置为 NullNode/NullPin
bool MaterialGraph::DisconnectInput(NodeID node, const std::string& pin)
{
	MaterialNode* target = GetNode(node);
	if (!target)
		return false;

	MaterialPin* inputPin = target->GetInputPin(pin);
	if (!inputPin)
		return false;

	inputPin->ConnectedNode = NullNode;
	inputPin->ConnectedPin = NullPin;
	return true;
}

// 拓扑排序：使用 Kahn 算法计算节点求值顺序
// 输出节点排在最后，其余节点按依赖顺序排列
std::vector<MaterialNode*> MaterialGraph::TopologicalSort() const
{
	// Kahn's algorithm

	// 计算每个节点的入度（已连接的输入引脚数量）
	std::unordered_map<NodeID, int32_t> inDegree;
	for (auto& node : m_Nodes)
	{
		int32_t degree = 0;
		for (auto& pin : node->GetInputPins())
		{
			if (pin.IsConnected())
				degree++;
		}
		inDegree[node->GetID()] = degree;
	}

	// 初始化队列：入度为 0 的节点（无依赖的节点）先入队
	std::queue<MaterialNode*> queue;
	for (auto& node : m_Nodes)
	{
		if (inDegree[node->GetID()] == 0)
			queue.push(node.get());
	}

	std::vector<MaterialNode*> result;
	std::vector<MaterialNode*> outputNodes;

	while (!queue.empty())
	{
		MaterialNode* current = queue.front();
		queue.pop();

		// 输出节点单独收集，最后追加到末尾
		if (current->GetTypeName() == "MaterialOutput")
			outputNodes.push_back(current);
		else
			result.push_back(current);

		// 遍历当前节点的每个输出引脚，查找所有连接到这些引脚的下游节点
		// 将下游节点的入度减 1，入度变为 0 的节点入队
		const auto& outputPins = current->GetOutputPins();
		for (size_t pinIdx = 0; pinIdx < outputPins.size(); ++pinIdx)
		{
			PinID thisPinID{ static_cast<uint32_t>(pinIdx) };

			for (auto& otherNode : m_Nodes)
			{
				for (auto& inputPin : otherNode->GetInputPins())
				{
					if (inputPin.IsConnected() &&
					    inputPin.ConnectedNode.Value == current->GetID().Value &&
					    inputPin.ConnectedPin.Value == thisPinID.Value)
					{
						NodeID downstreamID = otherNode->GetID();
						inDegree[downstreamID]--;
						if (inDegree[downstreamID] == 0)
							queue.push(otherNode.get());
					}
				}
			}
		}
	}

	// 将输出节点追加到排序结果的末尾
	result.insert(result.end(), outputNodes.begin(), outputNodes.end());

	return result;
}

// 检测环路：DFS 三色标记法
// 0 = White（未访问）, 1 = Gray（正在访问，检测到灰色节点即存在环）, 2 = Black（已处理完毕）
bool MaterialGraph::HasCycles() const
{
	// DFS three-color marking: 0 = White, 1 = Gray, 2 = Black
	std::unordered_map<NodeID, int32_t> color;
	for (auto& node : m_Nodes)
		color[node->GetID()] = 0;

	std::function<bool(MaterialNode*)> dfs = [&](MaterialNode* node) -> bool
	{
		NodeID id = node->GetID();
		if (color[id] == 1)
			return true; // cycle detected (gray node)
		if (color[id] == 2)
			return false; // already processed (black node)

		color[id] = 1; // mark gray

		// 沿输入引脚方向递归遍历上游节点
		for (auto& pin : node->GetInputPins())
		{
			if (pin.IsConnected())
			{
				MaterialNode* upstream = GetNode(pin.ConnectedNode);
				if (upstream && dfs(upstream))
					return true;
			}
		}

		color[id] = 2; // mark black
		return false;
	};

	// 对每个未访问的节点执行 DFS
	for (auto& node : m_Nodes)
	{
		if (color[node->GetID()] == 0)
		{
			if (dfs(node.get()))
				return true;
		}
	}

	return false;
}

// 验证图的完整性：
// 1. 不能有环
// 2. 必须有 MaterialOutput 节点
// 3. 所有连接的目标引脚必须有效且类型兼容
bool MaterialGraph::Validate() const
{
	if (HasCycles())
	{
		AF_LOG_WARN("MaterialGraph::Validate: graph contains cycles");
		return false;
	}

	if (!FindOutputNode())
	{
		AF_LOG_WARN("MaterialGraph::Validate: no MaterialOutput node found");
		return false;
	}

	// Validate all connections have compatible types
	for (auto& node : m_Nodes)
	{
		for (auto& inputPin : node->GetInputPins())
		{
			if (!inputPin.IsConnected())
				continue;

			// 检查连接的源节点是否存在
			MaterialNode* srcNode = GetNode(inputPin.ConnectedNode);
			if (!srcNode)
			{
				AF_LOG_WARN("MaterialGraph::Validate: dangling connection on pin '{}'", inputPin.Name);
				return false;
			}

			// 检查连接的引脚索引是否有效
			const auto& srcOutputs = srcNode->GetOutputPins();
			if (inputPin.ConnectedPin.Value >= srcOutputs.size())
			{
				AF_LOG_WARN("MaterialGraph::Validate: invalid pin index on connection");
				return false;
			}

			// 检查源引脚和目标引脚类型是否兼容
			if (!AreTypesCompatible(srcOutputs[inputPin.ConnectedPin.Value].Type, inputPin.Type))
			{
				AF_LOG_WARN("MaterialGraph::Validate: type mismatch on pin '{}'", inputPin.Name);
				return false;
			}
		}
	}

	return true;
}

// 分配并返回一个唯一的节点 ID（简单自增）
NodeID MaterialGraph::AllocateNodeID()
{
	return NodeID{ m_NextNodeID++ };
}

} // namespace AF
