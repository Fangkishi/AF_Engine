#include "Material/MaterialInstance.h"

namespace AF {

/// 合并父材质默认参数与实例覆盖参数，返回解析后的参数存储
const MaterialParameterStore& MaterialInstance::GetResolvedParameters() const
{
    if (m_CacheDirty)
    {
        if (Parent)
            m_ResolvedCache = Parent->DefaultParameters;
        else
            m_ResolvedCache = MaterialParameterStore{};

        m_ResolvedCache.MergeOverride(m_Overrides);
    }
    return m_ResolvedCache;
}

/// 录制材质 UBO + 纹理上传命令
void MaterialInstance::RecordBind(RHI::RHICommandBuffer& cmd, const ShaderReflection& reflection) const
{
    if (!m_MaterialUBO) return;

    if (m_CacheDirty)
    {
        const auto& resolved = GetResolvedParameters();
        const auto& descriptors = Parent ? Parent->ParameterDescriptors
                                         : std::vector<MaterialParameterDesc>{};
        m_CachedPackedUBO = resolved.PackUBO(descriptors);
        m_CacheDirty = false;
    }

    cmd.SetBufferData(m_MaterialUBO, m_CachedPackedUBO.data(),
                      static_cast<uint32_t>(m_CachedPackedUBO.size()), 0);
    cmd.BindUniformBuffer(m_MaterialUBO, 1);

    GetResolvedParameters().UploadTo(cmd, reflection);
}

} // namespace AF
