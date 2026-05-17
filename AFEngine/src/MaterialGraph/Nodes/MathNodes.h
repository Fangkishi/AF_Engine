#pragma once

// ============================================================================
// MathNodes.h — 数学运算节点
//
// 提供加/减/乘/除/幂/平方根/截断/饱和/最小/最大/绝对值/线性插值
// 共 12 种数学运算节点，输出类型默认 Float4 以支持向量运算。
// ============================================================================

#include "MaterialGraph/MaterialNode.h"

namespace AF {

class CompilerContext;

class AddNode : public MaterialNode
{
public:
	AddNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class SubtractNode : public MaterialNode
{
public:
	SubtractNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class MultiplyNode : public MaterialNode
{
public:
	MultiplyNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class DivideNode : public MaterialNode
{
public:
	DivideNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class PowerNode : public MaterialNode
{
public:
	PowerNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class SqrtNode : public MaterialNode
{
public:
	SqrtNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class ClampNode : public MaterialNode
{
public:
	ClampNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class SaturateNode : public MaterialNode
{
public:
	SaturateNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class MinNode : public MaterialNode
{
public:
	MinNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class MaxNode : public MaterialNode
{
public:
	MaxNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class AbsNode : public MaterialNode
{
public:
	AbsNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

class LerpNode : public MaterialNode
{
public:
	LerpNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

} // namespace AF
