#pragma once

// ============================================================================
// TextureNodes.h — 纹理采样节点
//
// 提供 TextureSampleNode：对 2D 纹理进行采样，输出 RGBA 及独立的 R/G/B/A 分量。
// ============================================================================

#include "MaterialGraph/MaterialNode.h"

#include <glm/glm.hpp>

namespace AF {

class CompilerContext;

// 纹理采样节点：采样 Texture2D，输出 vec4(RGBA) 和四个独立 float 分量
class TextureSampleNode : public MaterialNode
{
public:
	TextureSampleNode(NodeID id);

	std::string GenerateCode(CompilerContext& ctx) override;
	std::vector<std::string> GetVariantDefines() const override;
};

} // namespace AF
