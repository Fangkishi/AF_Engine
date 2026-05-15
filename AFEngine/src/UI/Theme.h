#pragma once

#include <string>
#include <imgui.h>

namespace AF {

struct ThemeStyle
{
    float WindowRounding      = 3.0f;
    float WindowBorderSize    = 0.0f;
    float ChildRounding       = 3.0f;
    float PopupRounding       = 3.0f;
    float FramePaddingX       = 8.0f;
    float FramePaddingY       = 4.0f;
    float FrameRounding       = 3.0f;
    float ItemSpacingX        = 8.0f;
    float ItemSpacingY        = 4.0f;
    float ItemInnerSpacingX   = 4.0f;
    float ItemInnerSpacingY   = 4.0f;
    float IndentSpacing       = 16.0f;
    float ScrollbarSize       = 14.0f;
    float ScrollbarRounding   = 3.0f;
    float GrabMinSize         = 10.0f;
    float GrabRounding        = 3.0f;
    float TabRounding         = 3.0f;
    float TabBorderSize       = 1.0f;
};

struct Theme
{
    std::string Name;
    std::string FontRegular;
    std::string FontBold;
    float       FontSize = 16.0f;

    ImVec4      Colors[ImGuiCol_COUNT];
    ThemeStyle  Style;

    static Theme CreateDefaultDark();
    static Theme CreateDefaultLight();
};

class AbstractThemeApplier
{
public:
    virtual ~AbstractThemeApplier() = default;

    virtual void ApplyColors(const ImVec4 colors[ImGuiCol_COUNT]) = 0;
    virtual void ApplyStyle(const ThemeStyle& style) = 0;
    virtual bool LoadFont(const std::string& regularPath,
                          const std::string& boldPath, float size) = 0;
};

} // namespace AF
