#pragma once

// ThemeSerializer —— 主题 JSON 序列化/反序列化
//
// 基于 nlohmann/json 实现 Theme ↔ JSON 双向转换。
// s_ColorMap 将 ImGuiCol_ 枚举名（如 "Text"）映射到 ImGuiCol_ 索引。

#include "UI/Theme.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>

namespace AF {

struct ThemeLoadResult
{
    std::optional<Theme>          Theme;
    std::string                   ErrorMessage;
    std::vector<std::string>      MissingFields;
};

class ThemeSerializer
{
public:
    static ThemeLoadResult Deserialize(const std::string& jsonText);
    static std::string Serialize(const Theme& theme);

    static const char* ColorNameFromIndex(int colIndex);
    static int ColorIndexFromName(const char* name);

private:
    struct ColorEntry { const char* Name; int Index; };
    static const ColorEntry s_ColorMap[];
};

} // namespace AF
