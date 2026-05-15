#pragma once

#include "Core/Types.h"

#include <glm/glm.hpp>
#include <initializer_list>
#include <string>

namespace AF {
namespace RHI {

enum class ShaderStage
{
    Vertex,
    Fragment,
};

struct ShaderSourceFile
{
    ShaderStage Stage;
    std::string Filepath;
};

class RHIShader
{
public:
    virtual ~RHIShader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetInt(const std::string& name, int value) = 0;
    virtual void SetFloat(const std::string& name, float value) = 0;
    virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
    virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
    virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

    virtual const std::string& GetName() const = 0;

    static Ref<RHIShader> Create(const std::string& name,
                                  std::initializer_list<ShaderSourceFile> sources);
};

} // namespace RHI
} // namespace AF
