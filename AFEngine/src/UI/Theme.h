#pragma once

// Theme —— 主题数据结构与 AbstractThemeApplier 虚接口
//
// Theme 包含名称、字体路径、颜色表和风格参数。
// AbstractThemeApplier 为生产（ImGuiThemeApplier）和测试（Mock）提供统一接口。

#include <string>
#include <imgui.h>

namespace AF {

/// 可序列化的 ImGui 风格参数
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

    /// 创建内置深色主题
    static Theme CreateDefaultDark();
    /// 创建内置浅色主题（当前为深色主题副本）
    static Theme CreateDefaultLight();
};

/// 主题应用器抽象接口 —— 支持生产实现和 Mock 测试
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
