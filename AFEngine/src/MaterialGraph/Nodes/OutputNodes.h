#pragma once

// ============================================================================
// OutputNodes.h — 材质输出节点
//
// 材质图的终端节点，定义了材质最终输出到着色器的各个属性：
// BaseColor, Metallic, Roughness, AmbientOcclusion, NormalWS,
// Emissive, OpacityMask, Opacity。
// ============================================================================

#include "MaterialGraph/MaterialNode.h"

#include <glm/glm.hpp>

namespace AF {

class CompilerContext;

// 材质输出节点：图中应有且仅有一个，作为材质编译的终点
class MaterialOutputNode : public MaterialNode
{
public:
	MaterialOutputNode(NodeID id);

	std::string GenerateCode(CompilerContext& ctx) override;
};

} // namespace AF
