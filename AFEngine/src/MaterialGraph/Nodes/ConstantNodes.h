#pragma once

// ============================================================================
// ConstantNodes.h — 常量值节点
//
// 提供 Scalar(标量)、Vector2/3/4(向量)、Color(颜色) 五种常量节点。
// 每个常量节点支持暴露为材质参数(从 UBO 读取)或编译为内联常量。
// ============================================================================

#include "MaterialGraph/MaterialNode.h"

#include <glm/glm.hpp>

namespace AF {

class CompilerContext;

// 标量常量节点：输出 float 值
class ScalarConstantNode : public MaterialNode
{
public:
	ScalarConstantNode(NodeID id, float defaultVal = 0.0f, bool exposeAsParam = false);

	std::string GenerateCode(CompilerContext& ctx) override;
};

// vec2 常量节点：输出 glm::vec2 值
class Vector2ConstantNode : public MaterialNode
{
public:
	Vector2ConstantNode(NodeID id, const glm::vec2& defaultVal = { 0.0f, 0.0f }, bool exposeAsParam = false);

	std::string GenerateCode(CompilerContext& ctx) override;
};

// vec3 常量节点：输出 glm::vec3 值
class Vector3ConstantNode : public MaterialNode
{
public:
	Vector3ConstantNode(NodeID id, const glm::vec3& defaultVal = { 0.0f, 0.0f, 0.0f }, bool exposeAsParam = false);

	std::string GenerateCode(CompilerContext& ctx) override;
};

// vec4 常量节点：输出 glm::vec4 值
class Vector4ConstantNode : public MaterialNode
{
public:
	Vector4ConstantNode(NodeID id, const glm::vec4& defaultVal = { 0.0f, 0.0f, 0.0f, 0.0f }, bool exposeAsParam = false);

	std::string GenerateCode(CompilerContext& ctx) override;
};

// 颜色常量节点：输出 vec3(Color3) 值，默认灰色 (0.5, 0.5, 0.5)
class ColorConstantNode : public MaterialNode
{
public:
	ColorConstantNode(NodeID id, const glm::vec3& defaultVal = { 0.5f, 0.5f, 0.5f }, bool exposeAsParam = false);

	std::string GenerateCode(CompilerContext& ctx) override;
};

} // namespace AF
