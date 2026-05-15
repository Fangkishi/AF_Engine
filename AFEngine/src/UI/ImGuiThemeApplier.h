#pragma once

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
