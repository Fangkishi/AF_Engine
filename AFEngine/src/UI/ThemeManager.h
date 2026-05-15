#pragma once

#include "UI/Theme.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace AF {

class AbstractThemeApplier;

class ThemeManager
{
public:
    static ThemeManager& Get();

    void Initialize(const std::string& themesDir,
                    std::unique_ptr<AbstractThemeApplier> applier);
    void Shutdown();

    bool ApplyTheme(const std::string& name);
    void ApplyTheme(const Theme& theme);

    const std::vector<std::string>& GetThemeNames() const;
    const Theme& GetCurrent() const;

private:
    ThemeManager() = default;

    void ScanThemes(const std::string& themesDir);

    std::unordered_map<std::string, Theme> m_Themes;
    std::vector<std::string>               m_ThemeNames;
    std::unique_ptr<AbstractThemeApplier>  m_Applier;
    Theme                                  m_Current;
    bool                                   m_Initialized = false;
};

} // namespace AF
