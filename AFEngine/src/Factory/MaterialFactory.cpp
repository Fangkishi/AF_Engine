#include "Factory/MaterialFactory.h"

#include "RHI/RHIShader.h"

namespace AF {

Ref<RHI::RHIShader> MaterialFactory::GetDefaultShader()
{
    static Ref<RHI::RHIShader> s_Shader = RHI::RHIShader::Create("default_material", {
        { RHI::ShaderStage::Vertex,   "assets/shaders/gbuffer.vert" },
        { RHI::ShaderStage::Fragment, "assets/shaders/gbuffer.frag" },
    });
    return s_Shader;
}

Ref<Material> MaterialFactory::CreateDefault()
{
    return std::make_shared<Material>(GetDefaultShader());
}

Ref<Material> MaterialFactory::CreateError()
{
    return std::make_shared<Material>(GetDefaultShader());
}

} // namespace AF
