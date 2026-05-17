#pragma once

// ============================================================================
// MaterialPin.h — 引脚类型与连接数据结构
//
// 功能：定义材质图中引脚(NodeID/PinID)、引脚枚举类型(MaterialPinType)、
//       类型兼容性检查、GLSL 类型映射以及 Std140 内存布局信息。
// ============================================================================

#include "Material/MaterialParameterStore.h"

#include <cstdint>
#include <functional>
#include <string>
#include <cstddef>

namespace AF {

// 节点 ID 和引脚 ID 的简单包装结构
struct NodeID { uint32_t Value = 0; };
struct PinID { uint32_t Value = 0; };

// 空节点/引脚标记值（UINT32_MAX 表示未连接）
constexpr NodeID NullNode{ UINT32_MAX };
constexpr PinID NullPin{ UINT32_MAX };

inline bool operator==(NodeID a, NodeID b) { return a.Value == b.Value; }
inline bool operator!=(NodeID a, NodeID b) { return a.Value != b.Value; }

// 材质图支持的引脚数据类型枚举
enum class MaterialPinType : uint8_t
{
	Float,        // float 标量
	Float2,       // vec2
	Float3,       // vec3
	Float4,       // vec4
	Color3,       // vec3（颜色语义，和 Float3 可互转）
	Color4,       // vec4（颜色语义，和 Float4 可互转）
	Texture2D,    // sampler2D
	TextureCube,  // samplerCube
	SamplerState, // 采样器状态
	Boolean,      // bool
	MaterialAttr  // 材质输出属性（用于 MaterialOutput 节点的输入引脚）
};

// 检查两个引脚类型是否可以连接
bool AreTypesCompatible(MaterialPinType src, MaterialPinType dst);
// 返回两个类型中维度更高的类型（用于自动提升）
MaterialPinType GetPromotedType(MaterialPinType a, MaterialPinType b);
// 将 MaterialPinType 映射为 GLSL 类型字符串
std::string ToGLSLType(MaterialPinType type);

// Std140 内存布局信息：GPU Uniform Buffer 的内存对齐描述
struct Std140Info
{
	std::string GlslType;  // GLSL 类型名
	size_t Size = 0;       // 占用字节数
	size_t Alignment = 0;  // 对齐要求
	bool IsVec3 = false;   // 是否为 vec3（std140 中需特殊处理）
};

// 获取指定引脚类型对应的 Std140 内存布局信息
Std140Info GetStd140Info(MaterialPinType type);

// ============================================================================
// MaterialPin — 引脚数据结构
//
// 代表节点上的一个输入或输出端口。Name 是引脚在节点内的唯一标识，
// Type 是数据类型，DefaultValue 是未连接时的默认值。
// ConnectedNode 和 ConnectedPin 用于记录引脚的连接目标。
// ============================================================================
struct MaterialPin
{
	std::string Name;                     // 引脚名称（如 "BaseColor", "UVs"）
	MaterialPinType Type = MaterialPinType::Float;  // 引脚数据类型
	MaterialParamValue DefaultValue;      // 未连接时的默认值

	NodeID ConnectedNode = NullNode;      // 连接到的源节点 ID（输入引脚使用）
	PinID ConnectedPin = NullPin;         // 连接到的源引脚索引（输入引脚使用）

	// 判断此引脚是否已连接到其他节点
	bool IsConnected() const { return ConnectedNode.Value != NullNode.Value; }
};

} // namespace AF

// 为 NodeID 提供 std::hash 特化，使其可用于 unordered_map/set
template<>
struct std::hash<AF::NodeID>
{
	size_t operator()(const AF::NodeID& id) const noexcept
	{
		return std::hash<uint32_t>{}(id.Value);
	}
};
