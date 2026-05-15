#pragma once

#include "Core/Types.h"
#include "RHI/RHIShader.h"
#include "RHI/RHICommandBuffer.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

namespace AF {

class Material
{
public:
    explicit Material(const Ref<RHI::RHIShader>& shader);

    void RecordBind(RHI::RHICommandBuffer& cmd) const;

    const Ref<RHI::RHIShader>& GetShader() const { return m_Shader; }

    Material& SetMat4(const std::string& name, const glm::mat4& value);
    Material& SetFloat4(const std::string& name, const glm::vec4& value);
    Material& SetFloat3(const std::string& name, const glm::vec3& value);
    Material& SetFloat(const std::string& name, float value);
    Material& SetInt(const std::string& name, int value);

private:
    Ref<RHI::RHIShader> m_Shader;
    std::unordered_map<std::string, glm::mat4> m_Mat4s;
    std::unordered_map<std::string, glm::vec4> m_Float4s;
    std::unordered_map<std::string, glm::vec3> m_Float3s;
    std::unordered_map<std::string, float> m_Floats;
    std::unordered_map<std::string, int> m_Ints;
};

} // namespace AF
