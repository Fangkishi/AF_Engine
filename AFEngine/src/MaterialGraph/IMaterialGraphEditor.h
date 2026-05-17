#pragma once

// ============================================================================
// IMaterialGraphEditor.h — 材质图编辑器接口
//
// 功能：定义材质图形编辑器的抽象接口。编辑器层实现此接口以提供 ImGui 渲染、
//       节点添加/删除/连接等交互操作，以及 GLSL 预览等调试功能。
// ============================================================================

#include "Core/Types.h"
#include "MaterialGraph/MaterialNode.h"
#include "MaterialGraph/NodeFactory.h"

#include <glm/glm.hpp>
#include <string>

namespace AF {

class MaterialGraph;

// ============================================================================
// IMaterialGraphEditor — 材质图编辑器虚接口
//
// 提供以下能力的抽象：
// - 读取/写入 MaterialGraph 数据
// - 添加/删除节点和图连接
// - ImGui 渲染回调
// - GLSL 代码预览
// - 节点类型注册
// ============================================================================
class IMaterialGraphEditor
{
public:
	virtual ~IMaterialGraphEditor() = default;
	// 从外部设置要编辑的材质图
	virtual void SetGraph(const MaterialGraph& graph) = 0;
	// 获取当前编辑的材质图
	virtual MaterialGraph GetGraph() const = 0;
	// 在指定位置添加一个新节点
	virtual NodeID AddNode(const std::string& typeName, glm::vec2 position) = 0;
	// 删除指定节点
	virtual void RemoveNode(NodeID id) = 0;
	// 在两个节点引脚之间建立连接
	virtual bool Connect(NodeID srcNode, const std::string& srcPin,
	                     NodeID dstNode, const std::string& dstPin) = 0;
	// 断开指定节点的输入引脚
	virtual void DisconnectInput(NodeID node, const std::string& pin) = 0;
	// 渲染编辑器 UI（ImGui 帧调用）
	virtual void OnImGuiRender() = 0;
	// 生成并返回 GLSL 预览代码字符串
	virtual std::string PreviewGLSL(const std::string& materialName) = 0;
	// 注册一个节点类型到编辑器的创建菜单
	virtual void RegisterNodeType(const std::string& name, NodeFactory::Creator factory) = 0;
};

} // namespace AF
