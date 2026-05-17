#pragma once

// MaterialFactory —— 材质工厂
//
// 创建默认材质和错误材质（粉红色显眼标记，方便调试）。

#include "Core/Types.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"

namespace AF {

class MaterialFactory
{
public:
    static Ref<Material> CreateErrorMaterial();
    static Ref<MaterialInstance> CreateError();
    static Ref<MaterialInstance> CreateDefault();
};

} // namespace AF
