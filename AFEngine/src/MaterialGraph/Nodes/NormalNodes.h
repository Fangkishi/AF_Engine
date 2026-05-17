#pragma once

// ============================================================================
// NormalNodes.h — 法线计算节点
//
// 提供从高度图生成法线(NormalFromHeightmap)和
// 角度校正法线混合(BlendAngleCorrectedNormals)两种法线处理节点。
// ============================================================================

#include "MaterialGraph/MaterialNode.h"

#include <glm/glm.hpp>

namespace AF {

class CompilerContext;

// 从高度图生成法线贴图：通过对高度图纹理进行有限差分计算法线方向
class NormalFromHeightmapNode : public MaterialNode
{
public:
	NormalFromHeightmapNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

// 角度校正法线混合：采用 UDN 混合算法将细节法线叠加到基础法线上
class BlendAngleCorrectedNormalsNode : public MaterialNode
{
public:
	BlendAngleCorrectedNormalsNode(NodeID id);
	std::string GenerateCode(CompilerContext& ctx) override;
};

} // namespace AF
