#pragma once

// ThemeManager —— 主题管理器（单例）
//
// 扫描 Resources/Themes/ 目录下的 JSON 主题文件，
// 通过 AbstractThemeApplier 将主题应用到 ImGui 样式和字体。
// 支持按名称切换和内置降级默认主题。

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
