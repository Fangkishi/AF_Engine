#pragma once

#include "RHI/RHIShader.h"

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace AF {
namespace RHI {

class GLShader : public RHIShader
{
public:
    GLShader(const std::string& name, const std::vector<ShaderSourceFile>& sources);
    ~GLShader() override;

    void Bind() const override;
    void Unbind() const override;

    void SetInt(const std::string& name, int value) override;
    void SetFloat(const std::string& name, float value) override;
    void SetFloat3(const std::string& name, const glm::vec3& value) override;
    void SetFloat4(const std::string& name, const glm::vec4& value) override;
    void SetMat4(const std::string& name, const glm::mat4& value) override;

    const std::string& GetName() const override { return m_Name; }

private:
    static unsigned int StageToGL(ShaderStage stage);
    std::string ReadFile(const std::string& filepath);
    void Compile(const std::unordered_map<unsigned int, std::string>& shaderSources);
    int GetUniformLocation(const std::string& name) const;

    uint32_t m_RendererID = 0;
    std::string m_Name;
    mutable std::unordered_map<std::string, int> m_UniformLocationCache;
};

} // namespace RHI
} // namespace AF
