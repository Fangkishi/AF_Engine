#include "Material/MaterialParameterStore.h"
#include "MaterialGraph/MaterialCompiler.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <unordered_set>

namespace AF {

void MaterialParameterStore::Set(const std::string& name, float v)
{
    m_Params[name] = v;
}

void MaterialParameterStore::Set(const std::string& name, const glm::vec2& v)
{
    m_Params[name] = v;
}

void MaterialParameterStore::Set(const std::string& name, const glm::vec3& v)
{
    m_Params[name] = v;
}

void MaterialParameterStore::Set(const std::string& name, const glm::vec4& v)
{
    m_Params[name] = v;
}

void MaterialParameterStore::Set(const std::string& name, int32_t v)
{
    m_Params[name] = v;
}

void MaterialParameterStore::Set(const std::string& name, bool v)
{
    m_Params[name] = v;
}

void MaterialParameterStore::Set(const std::string& name, const Ref<RHI::RHITexture2D>& v)
{
    m_Params[name] = v;
}

void MaterialParameterStore::Set(const std::string& name, const Ref<RHI::RHITextureCube>& v)
{
    m_Params[name] = v;
}

bool MaterialParameterStore::Get(const std::string& name, float& out) const
{
    auto it = m_Params.find(name);
    if (it != m_Params.end() && std::holds_alternative<float>(it->second))
    {
        out = std::get<float>(it->second);
        return true;
    }
    return false;
}

bool MaterialParameterStore::Get(const std::string& name, glm::vec2& out) const
{
    auto it = m_Params.find(name);
    if (it != m_Params.end() && std::holds_alternative<glm::vec2>(it->second))
    {
        out = std::get<glm::vec2>(it->second);
        return true;
    }
    return false;
}

bool MaterialParameterStore::Get(const std::string& name, glm::vec3& out) const
{
    auto it = m_Params.find(name);
    if (it != m_Params.end() && std::holds_alternative<glm::vec3>(it->second))
    {
        out = std::get<glm::vec3>(it->second);
        return true;
    }
    return false;
}

bool MaterialParameterStore::Get(const std::string& name, glm::vec4& out) const
{
    auto it = m_Params.find(name);
    if (it != m_Params.end() && std::holds_alternative<glm::vec4>(it->second))
    {
        out = std::get<glm::vec4>(it->second);
        return true;
    }
    return false;
}

bool MaterialParameterStore::Get(const std::string& name, int32_t& out) const
{
    auto it = m_Params.find(name);
    if (it != m_Params.end() && std::holds_alternative<int32_t>(it->second))
    {
        out = std::get<int32_t>(it->second);
        return true;
    }
    return false;
}

bool MaterialParameterStore::Get(const std::string& name, bool& out) const
{
    auto it = m_Params.find(name);
    if (it != m_Params.end() && std::holds_alternative<bool>(it->second))
    {
        out = std::get<bool>(it->second);
        return true;
    }
    return false;
}

bool MaterialParameterStore::Get(const std::string& name, Ref<RHI::RHITexture2D>& out) const
{
    auto it = m_Params.find(name);
    if (it != m_Params.end() && std::holds_alternative<Ref<RHI::RHITexture2D>>(it->second))
    {
        out = std::get<Ref<RHI::RHITexture2D>>(it->second);
        return true;
    }
    return false;
}

bool MaterialParameterStore::Get(const std::string& name, Ref<RHI::RHITextureCube>& out) const
{
    auto it = m_Params.find(name);
    if (it != m_Params.end() && std::holds_alternative<Ref<RHI::RHITextureCube>>(it->second))
    {
        out = std::get<Ref<RHI::RHITextureCube>>(it->second);
        return true;
    }
    return false;
}

bool MaterialParameterStore::Contains(const std::string& name) const
{
    return m_Params.find(name) != m_Params.end();
}

size_t MaterialParameterStore::Size() const
{
    return m_Params.size();
}

void MaterialParameterStore::MergeOverride(const MaterialParameterStore& overrides)
{
    for (const auto& [key, value] : overrides.m_Params)
    {
        auto it = m_Params.find(key);
        if (it != m_Params.end())
            it->second = value;
    }
}

void MaterialParameterStore::UploadTo(RHI::RHICommandBuffer& cmd, const ShaderReflection& reflection) const
{
    for (const auto& [name, value] : m_Params)
    {
        std::visit(RHI::Overloaded{
            [&](const Ref<RHI::RHITexture2D>& tex) {
                uint32_t slot = reflection.GetTextureBinding(name);
                if (slot != UINT32_MAX && tex)
                    cmd.BindTexture(slot, tex);
            },
            [&](const Ref<RHI::RHITextureCube>& tex) {
                uint32_t slot = reflection.GetTextureBinding(name);
                if (slot != UINT32_MAX && tex)
                    cmd.BindTextureCube(slot, tex);
            },
            [&](auto) {},
        }, value);
    }
}

std::vector<uint8_t> MaterialParameterStore::PackUBO(const std::vector<MaterialParameterDesc>& descriptors) const
{
	std::unordered_set<std::string> seen;

	auto addDesc = [&](std::vector<MaterialParameterDesc>& out, const MaterialParameterDesc& d) {
		if (seen.count(d.Name)) return;
		seen.insert(d.Name);
		out.push_back(d);
	};

	std::vector<MaterialParameterDesc> allDescs;

	for (const auto& d : descriptors)
	{
		auto info = GetStd140Info(d.Type);
		if (info.GlslType.empty()) continue;
		addDesc(allDescs, d);
	}

	// Calculate total size
	size_t totalSize = 0;
	for (const auto& d : allDescs)
	{
		auto info = GetStd140Info(d.Type);
		totalSize = (totalSize + info.Alignment - 1) / info.Alignment * info.Alignment + info.Size;
	}

	std::vector<uint8_t> data(totalSize, 0);

	size_t offset = 0;
	for (const auto& d : allDescs)
	{
		auto info = GetStd140Info(d.Type);
		offset = (offset + info.Alignment - 1) / info.Alignment * info.Alignment;

		// Get value: prefer map, fallback to descriptor default
		const auto* val = &d.DefaultValue;
		const MaterialParamValue* mapVal = nullptr;
		auto it = m_Params.find(d.Name);
		if (it != m_Params.end())
			mapVal = &it->second;
		if (mapVal)
			val = mapVal;

		std::visit([&](auto&& v) {
			using T = std::decay_t<decltype(v)>;
			if constexpr (std::is_same_v<T, float>) {
				std::memcpy(data.data() + offset, &v, 4);
			} else if constexpr (std::is_same_v<T, glm::vec2>) {
				std::memcpy(data.data() + offset, glm::value_ptr(v), 8);
			} else if constexpr (std::is_same_v<T, glm::vec3>) {
				if (info.IsVec3) {
					// Pack as vec4 with w=1.0
					float packed[4] = { v.x, v.y, v.z, 1.0f };
					std::memcpy(data.data() + offset, packed, 16);
				} else {
					std::memcpy(data.data() + offset, glm::value_ptr(v), 12);
				}
			} else if constexpr (std::is_same_v<T, glm::vec4>) {
				if (info.IsVec3) {
					// Truncate to vec3 but still occupy 16 bytes
					float packed[4] = { v.x, v.y, v.z, 1.0f };
					std::memcpy(data.data() + offset, packed, 16);
				} else {
					std::memcpy(data.data() + offset, glm::value_ptr(v), 16);
				}
			} else if constexpr (std::is_same_v<T, int32_t>) {
				std::memcpy(data.data() + offset, &v, 4);
			} else if constexpr (std::is_same_v<T, bool>) {
				float fv = v ? 1.0f : 0.0f;
				std::memcpy(data.data() + offset, &fv, 4);
			}
		}, *val);

		offset += info.Size;
	}

	return data;
}

} // namespace AF
