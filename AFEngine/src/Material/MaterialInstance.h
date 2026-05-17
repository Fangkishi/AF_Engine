#pragma once

// MaterialInstance —— 材质实例（运行时挂载到实体）
//
// 每个实例包含对 Parent Material 的引用和覆盖参数（m_Overrides）。
// GetResolvedParameters 合并 Parent 默认参数与实例覆盖参数。
// RecordBind 将材质 UBO 数据上传到 GPU。

#include "Core/Types.h"
#include "Core/UUID.h"
#include "Material/Material.h"
#include "Material/MaterialParameterStore.h"
#include "RHI/ShaderReflection.h"
#include "RHI/RHIUniformBuffer.h"

#include <cstdint>
#include <vector>

namespace AF {

class MaterialInstance
{
public:
    MaterialInstance() = default;
    explicit MaterialInstance(const Ref<Material>& parent) : Parent(parent) {}

    template<typename T>
    void SetParameter(const std::string& name, const T& value)
    {
        m_Overrides.Set(name, value);
        m_CacheDirty = true;
    }

    template<typename T>
    void SetTexture(const std::string& name, const T& value)
    {
        m_Overrides.Set(name, value);
        m_CacheDirty = true;
    }

    void SetMaterialUBO(RHI::RHIUniformBuffer* ubo) { m_MaterialUBO = ubo; }

    const MaterialParameterStore& GetResolvedParameters() const;
    void RecordBind(RHI::RHICommandBuffer& cmd, const ShaderReflection& reflection) const;

    UUID AssetID;
    std::string Name;
    Ref<Material> Parent;

private:
    MaterialParameterStore m_Overrides;
    mutable MaterialParameterStore m_ResolvedCache;
    mutable std::vector<uint8_t> m_CachedPackedUBO;
    mutable bool m_CacheDirty = true;
    RHI::RHIUniformBuffer* m_MaterialUBO = nullptr;
};

} // namespace AF
