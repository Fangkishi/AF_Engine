#pragma once

// ============================================================================
// UtilityNodes.h — 工具节点
//
// 提供菲涅尔(Fresnel)、时间(Time)、纹理坐标(TexCoord)、
// 平移(Panner)、旋转(Rotator) 五种常用的材质工具节点。
// ============================================================================

#include "MaterialGraph/MaterialNode.h"

#include <glm/glm.hpp>

namespace AF {

class CompilerContext;

// 菲涅尔节点：基于法线与视线夹角计算边缘光效果
class FresnelNode : public MaterialNode
{
public:
	FresnelNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 时间节点：输出 frameData.Time 用于动画
class TimeNode : public MaterialNode
{
public:
	TimeNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 纹理坐标节点：选择 UV0 或 UV1 坐标集
class TexCoordNode : public MaterialNode
{
public:
	TexCoordNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 平移节点：随时间自动平移 UV 坐标（UV 动画）
class PannerNode : public MaterialNode
{
public:
	PannerNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 旋转节点：围绕中心点旋转 UV 坐标
class RotatorNode : public MaterialNode
{
public:
	RotatorNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

} // namespace AF
