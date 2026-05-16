#pragma once

#include "Core/Types.h"
#include "Renderer/Material.h"

namespace AF {

class MaterialFactory
{
public:
    static Ref<Material> CreateDefault();
    static Ref<Material> CreateError();

private:
    static Ref<RHI::RHIShader> GetDefaultShader();
};

} // namespace AF
