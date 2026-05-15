#pragma once

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
