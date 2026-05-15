#include "Renderer/Material.h"

namespace AF {

Material::Material(const Ref<RHI::RHIShader>& shader)
    : m_Shader(shader)
{
}

void Material::RecordBind(RHI::RHICommandBuffer& cmd) const
{
    cmd.BindShader(m_Shader);

    for (const auto& [name, value] : m_Mat4s)
        cmd.SetMat4(name, value);
    for (const auto& [name, value] : m_Float4s)
        cmd.SetFloat4(name, value);
    for (const auto& [name, value] : m_Float3s)
        cmd.SetFloat3(name, value);
    for (const auto& [name, value] : m_Floats)
        cmd.SetFloat(name, value);
    for (const auto& [name, value] : m_Ints)
        cmd.SetInt(name, value);
}

Material& Material::SetMat4(const std::string& name, const glm::mat4& value)   { m_Mat4s[name] = value; return *this; }
Material& Material::SetFloat4(const std::string& name, const glm::vec4& value) { m_Float4s[name] = value; return *this; }
Material& Material::SetFloat3(const std::string& name, const glm::vec3& value) { m_Float3s[name] = value; return *this; }
Material& Material::SetFloat(const std::string& name, float value)             { m_Floats[name] = value; return *this; }
Material& Material::SetInt(const std::string& name, int value)                 { m_Ints[name] = value; return *this; }

} // namespace AF
