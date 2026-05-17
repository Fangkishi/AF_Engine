#pragma once

// Material —— 材质资源（主素材）
//
// 包含材质名称、域（Surface/PostProcess/Decal）、混合模式、
// 着色模型、默认参数、纹理槽、材质图（MaterialGraph）和编译后的 Snippet。
// ParameterDescriptors 由 MaterialCompiler 编译时填充。

#include "Core/Types.h"
#include "Core/UUID.h"
#include "Material/MaterialDefines.h"
#include "Material/MaterialParameterStore.h"
#include "MaterialGraph/MaterialGraph.h"
#include "MaterialGraph/MaterialCompiler.h"

#include <string>
#include <vector>

namespace AF {

class Material
{
public:
    Material() = default;

    MaterialDomain GetDomain() const { return Domain; }
    MaterialBlendMode GetBlendMode() const { return BlendMode; }

    UUID AssetID;
    std::string Name;
    MaterialDomain Domain = MaterialDomain::Surface;
    MaterialBlendMode BlendMode = MaterialBlendMode::Opaque;
    std::string ShadingModel = "DefaultLit";
    MaterialParameterStore DefaultParameters;
    std::vector<MaterialParameterDesc> ParameterDescriptors;
    std::vector<TextureSlot> TextureSlots;
    MaterialGraph Graph;
    ShaderSnippet CompiledSnippet;
};

} // namespace AF
