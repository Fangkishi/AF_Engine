#pragma once

// ============================================================================
// VectorNodes.h — 向量运算节点
//
// 提供点积(DotProduct)、叉积(CrossProduct)、归一化(Normalize)、
// 向量拼接(AppendVector)、分量掩码(ComponentMask) 五种向量操作节点。
// ============================================================================

#include "MaterialGraph/MaterialNode.h"

#include <glm/glm.hpp>

namespace AF {

class CompilerContext;

// 点积节点：计算两个向量的点积，返回标量
class DotProductNode : public MaterialNode
{
public:
	DotProductNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 叉积节点：计算两个 vec3 的叉积，返回 vec3
class CrossProductNode : public MaterialNode
{
public:
	CrossProductNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 归一化节点：将输入向量归一化为单位向量
class NormalizeNode : public MaterialNode
{
public:
	NormalizeNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 向量拼接节点：将 vec2(A) 和 float(B) 拼接为 vec3
class AppendVectorNode : public MaterialNode
{
public:
	AppendVectorNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 分量掩码节点：通过 RGBA 掩码字符串选择向量的分量子集
// 如 "RGB" → vec3, "R" → float, "RGBA" → vec4
class ComponentMaskNode : public MaterialNode
{
public:
	ComponentMaskNode(NodeID id, const std::string& mask = "RGB");

	std::string GenerateCode(CompilerContext& ctx) override;
};

} // namespace AF
