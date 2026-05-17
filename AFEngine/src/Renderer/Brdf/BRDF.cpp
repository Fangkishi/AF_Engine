#include "Renderer/Brdf/BRDF.h"

namespace AF {

std::unordered_map<std::string, Ref<BRDF>>& BRDF::GetRegistry()
{
    static std::unordered_map<std::string, Ref<BRDF>> registry;
    return registry;
}

void BRDF::Register(const std::string& name, Ref<BRDF> brdf)
{
    GetRegistry()[name] = std::move(brdf);
}

Ref<BRDF> BRDF::Get(const std::string& name)
{
    auto& reg = GetRegistry();
    auto it = reg.find(name);
    return (it != reg.end()) ? it->second : nullptr;
}

std::vector<std::string> BRDF::GetAllNames()
{
    std::vector<std::string> names;
    for (const auto& [name, _] : GetRegistry())
        names.push_back(name);
    return names;
}

} // namespace AF
