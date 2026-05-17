#pragma once

// ImGuiThemeApplier —— 主题应用器的 ImGui 生产实现
//
// 将 Theme 的 Colors 和 Style 直接写入 ImGui::GetStyle()，
// LoadFont 从磁盘加载 TTF 字体文件，失败时降级到默认字体。

#include "UI/Theme.h"

namespace AF {

class ImGuiThemeApplier : public AbstractThemeApplier
{
public:
    void ApplyColors(const ImVec4 colors[ImGuiCol_COUNT]) override;
    void ApplyStyle(const ThemeStyle& style) override;
    bool LoadFont(const std::string& regularPath,
                  const std::string& boldPath, float size) override;
};

} // namespace AF
