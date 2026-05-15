#include "UI/ThemeManager.h"
#include "UI/ThemeSerializer.h"

#include "Core/Log.h"

#include <filesystem>

namespace AF {

static void ApplyViewportAdjustments()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
}

ThemeManager& ThemeManager::Get()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::Initialize(const std::string& themesDir,
                              std::unique_ptr<AbstractThemeApplier> applier)
{
    m_Applier    = std::move(applier);
    m_Initialized = true;

    ScanThemes(themesDir);

    if (m_Themes.empty())
    {
        Theme builtin = Theme::CreateDefaultDark();
        builtin.Name = "Built-in Dark";
        m_Themes[builtin.Name] = builtin;
        m_ThemeNames.push_back(builtin.Name);
        AF_LOG_INFO("ThemeManager: no themes found, registered built-in '{}'", builtin.Name);
    }
}

void ThemeManager::Shutdown()
{
    m_Themes.clear();
    m_ThemeNames.clear();
    m_Applier.reset();
    m_Initialized = false;
}

void ThemeManager::ScanThemes(const std::string& themesDir)
{
    std::filesystem::path dir(themesDir);
    std::error_code ec;

    if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec))
    {
        AF_LOG_WARN("ThemeManager: themes directory '{}' does not exist", themesDir);
        return;
    }

    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
    {
        if (ec) break;

        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        std::ifstream file(entry.path());
        if (!file.is_open())
        {
            AF_LOG_ERROR("ThemeManager: cannot open '{}'", entry.path().string());
            continue;
        }

        std::string jsonText((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

        ThemeLoadResult result = ThemeSerializer::Deserialize(jsonText);

        if (!result.ErrorMessage.empty())
        {
            AF_LOG_ERROR("ThemeManager: failed to parse '{}': {}",
                         entry.path().filename().string(), result.ErrorMessage);
            AF_LOG_WARN("ThemeManager: using built-in default for '{}'",
                         entry.path().filename().string());
        }

        for (const auto& field : result.MissingFields)
            AF_LOG_WARN("ThemeManager: missing field '{}' in '{}'",
                        field, entry.path().filename().string());

        if (result.Theme.has_value())
        {
            Theme& t = result.Theme.value();
            if (t.Name.empty())
                t.Name = entry.path().stem().string();
            m_Themes[t.Name] = t;
            m_ThemeNames.push_back(t.Name);
            ++count;
        }
    }

    AF_LOG_INFO("ThemeManager: loaded {} theme(s) from '{}'", count, themesDir);
}

bool ThemeManager::ApplyTheme(const std::string& name)
{
    auto it = m_Themes.find(name);
    if (it == m_Themes.end())
    {
        AF_LOG_WARN("ThemeManager: theme '{}' not registered", name);
        return false;
    }

    ApplyTheme(it->second);
    return true;
}

void ThemeManager::ApplyTheme(const Theme& theme)
{
    if (!m_Applier)
    {
        AF_LOG_ERROR("ThemeManager: applier not set, cannot apply theme");
        return;
    }

    m_Current = theme;

    m_Applier->ApplyColors(theme.Colors);
    m_Applier->ApplyStyle(theme.Style);

    bool fontOk = m_Applier->LoadFont(theme.FontRegular, theme.FontBold, theme.FontSize);
    if (!fontOk)
        AF_LOG_WARN("ThemeManager: regular font '{}' not found, using fallback", theme.FontRegular);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ApplyViewportAdjustments();

    AF_LOG_INFO("ThemeManager: applied theme '{}'", theme.Name);
}

const std::vector<std::string>& ThemeManager::GetThemeNames() const
{
    return m_ThemeNames;
}

const Theme& ThemeManager::GetCurrent() const
{
    return m_Current;
}

} // namespace AF
