#pragma once

#include "Core/Types.h"
#include "RHI/RHITexture.h"
#include "RHI/RHICommandBuffer.h"
#include "RHI/ShaderReflection.h"

#include <glm/glm.hpp>
#include <variant>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <vector>

namespace AF {

struct MaterialParameterDesc;

using MaterialParamValue = std::variant<
    float, glm::vec2, glm::vec3, glm::vec4,
    int32_t, bool,
    Ref<RHI::RHITexture2D>, Ref<RHI::RHITextureCube>
>;

class MaterialParameterStore
{
public:
    void Set(const std::string& name, float v);
    void Set(const std::string& name, const glm::vec2& v);
    void Set(const std::string& name, const glm::vec3& v);
    void Set(const std::string& name, const glm::vec4& v);
    void Set(const std::string& name, int32_t v);
    void Set(const std::string& name, bool v);
    void Set(const std::string& name, const Ref<RHI::RHITexture2D>& v);
    void Set(const std::string& name, const Ref<RHI::RHITextureCube>& v);

    bool Get(const std::string& name, float& out) const;
    bool Get(const std::string& name, glm::vec2& out) const;
    bool Get(const std::string& name, glm::vec3& out) const;
    bool Get(const std::string& name, glm::vec4& out) const;
    bool Get(const std::string& name, int32_t& out) const;
    bool Get(const std::string& name, bool& out) const;
    bool Get(const std::string& name, Ref<RHI::RHITexture2D>& out) const;
    bool Get(const std::string& name, Ref<RHI::RHITextureCube>& out) const;

    bool Contains(const std::string& name) const;
    size_t Size() const;

    const auto& GetMap() const { return m_Params; }

    void MergeOverride(const MaterialParameterStore& overrides);
    void UploadTo(RHI::RHICommandBuffer& cmd, const ShaderReflection& reflection) const;
    std::vector<uint8_t> PackUBO(const std::vector<MaterialParameterDesc>& descriptors) const;

private:
    std::unordered_map<std::string, MaterialParamValue> m_Params;
};

} // namespace AF
