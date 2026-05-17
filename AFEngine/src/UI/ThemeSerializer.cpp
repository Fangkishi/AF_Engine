#include "UI/ThemeSerializer.h"

#include "Core/Log.h"

#include <nlohmann/json.hpp>

namespace AF {

const ThemeSerializer::ColorEntry ThemeSerializer::s_ColorMap[] = {
    { "Text",                  ImGuiCol_Text                  },
    { "TextDisabled",          ImGuiCol_TextDisabled          },
    { "WindowBg",              ImGuiCol_WindowBg              },
    { "ChildBg",               ImGuiCol_ChildBg               },
    { "PopupBg",               ImGuiCol_PopupBg               },
    { "Border",                ImGuiCol_Border                },
    { "BorderShadow",          ImGuiCol_BorderShadow          },
    { "FrameBg",               ImGuiCol_FrameBg               },
    { "FrameBgHovered",        ImGuiCol_FrameBgHovered        },
    { "FrameBgActive",         ImGuiCol_FrameBgActive         },
    { "TitleBg",               ImGuiCol_TitleBg               },
    { "TitleBgActive",         ImGuiCol_TitleBgActive         },
    { "TitleBgCollapsed",      ImGuiCol_TitleBgCollapsed      },
    { "MenuBarBg",             ImGuiCol_MenuBarBg             },
    { "ScrollbarBg",           ImGuiCol_ScrollbarBg           },
    { "ScrollbarGrab",         ImGuiCol_ScrollbarGrab         },
    { "ScrollbarGrabHovered",  ImGuiCol_ScrollbarGrabHovered  },
    { "ScrollbarGrabActive",   ImGuiCol_ScrollbarGrabActive   },
    { "CheckMark",             ImGuiCol_CheckMark             },
    { "SliderGrab",            ImGuiCol_SliderGrab            },
    { "SliderGrabActive",      ImGuiCol_SliderGrabActive      },
    { "Button",                ImGuiCol_Button                },
    { "ButtonHovered",         ImGuiCol_ButtonHovered         },
    { "ButtonActive",          ImGuiCol_ButtonActive          },
    { "Header",                ImGuiCol_Header                },
    { "HeaderHovered",         ImGuiCol_HeaderHovered         },
    { "HeaderActive",          ImGuiCol_HeaderActive          },
    { "Separator",             ImGuiCol_Separator             },
    { "SeparatorHovered",      ImGuiCol_SeparatorHovered      },
    { "SeparatorActive",       ImGuiCol_SeparatorActive       },
    { "ResizeGrip",            ImGuiCol_ResizeGrip            },
    { "ResizeGripHovered",     ImGuiCol_ResizeGripHovered     },
    { "ResizeGripActive",      ImGuiCol_ResizeGripActive      },
    { "Tab",                   ImGuiCol_Tab                   },
    { "TabHovered",            ImGuiCol_TabHovered            },
    { "TabActive",             ImGuiCol_TabActive             },
    { "TabUnfocused",          ImGuiCol_TabUnfocused          },
    { "TabUnfocusedActive",    ImGuiCol_TabUnfocusedActive    },
    { "DockingPreview",        ImGuiCol_DockingPreview        },
    { "DockingEmptyBg",        ImGuiCol_DockingEmptyBg        },
    { "PlotLines",             ImGuiCol_PlotLines             },
    { "PlotLinesHovered",      ImGuiCol_PlotLinesHovered      },
    { "PlotHistogram",         ImGuiCol_PlotHistogram         },
    { "PlotHistogramHovered",  ImGuiCol_PlotHistogramHovered  },
    { "TableHeaderBg",         ImGuiCol_TableHeaderBg         },
    { "TableBorderStrong",     ImGuiCol_TableBorderStrong     },
    { "TableBorderLight",      ImGuiCol_TableBorderLight      },
    { "TableRowBg",            ImGuiCol_TableRowBg            },
    { "TableRowBgAlt",         ImGuiCol_TableRowBgAlt         },
    { "TextSelectedBg",        ImGuiCol_TextSelectedBg        },
    { "DragDropTarget",        ImGuiCol_DragDropTarget        },
    { "NavHighlight",          ImGuiCol_NavHighlight          },
    { "NavWindowingHighlight", ImGuiCol_NavWindowingHighlight },
    { "NavWindowingDimBg",     ImGuiCol_NavWindowingDimBg     },
    { "ModalWindowDimBg",      ImGuiCol_ModalWindowDimBg      },
};

const char* ThemeSerializer::ColorNameFromIndex(int colIndex)
{
    if (colIndex < 0 || colIndex >= ImGuiCol_COUNT)
        return "";
    return s_ColorMap[colIndex].Name;
}

int ThemeSerializer::ColorIndexFromName(const char* name)
{
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        if (strcmp(s_ColorMap[i].Name, name) == 0)
            return s_ColorMap[i].Index;
    }
    return -1;
}

static ImVec4 ParseColor(const nlohmann::json& arr)
{
    ImVec4 c = {};
    if (arr.is_array())
    {
        if (arr.size() > 0) c.x = arr[0].get<float>();
        if (arr.size() > 1) c.y = arr[1].get<float>();
        if (arr.size() > 2) c.z = arr[2].get<float>();
        if (arr.size() > 3) c.w = arr[3].get<float>();
    }
    return c;
}

static void ParseStyleFloat(const nlohmann::json& obj, const char* key, float& out,
                            std::vector<std::string>& missing)
{
    if (obj.contains(key))
        out = obj[key].get<float>();
    else
        missing.push_back(std::string("Style.") + key);
}

static void SerializeColor(nlohmann::json& obj, const char* key, const ImVec4& c)
{
    obj[key] = { c.x, c.y, c.z, c.w };
}

ThemeLoadResult ThemeSerializer::Deserialize(const std::string& jsonText)
{
    ThemeLoadResult result;
    result.Theme = Theme::CreateDefaultDark();

    if (jsonText.empty())
    {
        result.ErrorMessage = "Empty JSON text";
        return result;
    }

    try
    {
        nlohmann::json j = nlohmann::json::parse(jsonText);

        if (j.contains("Name") && j["Name"].is_string())
            result.Theme->Name = j["Name"].get<std::string>();

        if (j.contains("Font"))
        {
            auto& f = j["Font"];
            if (f.contains("Regular") && f["Regular"].is_string())
                result.Theme->FontRegular = f["Regular"].get<std::string>();
            else
                result.MissingFields.push_back("Font.Regular");

            if (f.contains("Bold") && f["Bold"].is_string())
                result.Theme->FontBold = f["Bold"].get<std::string>();
            else
                result.MissingFields.push_back("Font.Bold");

            if (f.contains("Size") && f["Size"].is_number() && f["Size"].get<float>() > 0.0f)
                result.Theme->FontSize = f["Size"].get<float>();
        }

        if (j.contains("Colors") && j["Colors"].is_object())
        {
            for (auto& [key, val] : j["Colors"].items())
            {
                int idx = ColorIndexFromName(key.c_str());
                if (idx >= 0)
                    result.Theme->Colors[idx] = ParseColor(val);
                else
                    result.MissingFields.push_back(std::string("Colors.") + key);
            }
        }

        if (j.contains("Style") && j["Style"].is_object())
        {
            auto& s = j["Style"];
            ParseStyleFloat(s, "WindowRounding",      result.Theme->Style.WindowRounding,      result.MissingFields);
            ParseStyleFloat(s, "WindowBorderSize",    result.Theme->Style.WindowBorderSize,    result.MissingFields);
            ParseStyleFloat(s, "ChildRounding",       result.Theme->Style.ChildRounding,       result.MissingFields);
            ParseStyleFloat(s, "PopupRounding",       result.Theme->Style.PopupRounding,       result.MissingFields);
            ParseStyleFloat(s, "FrameRounding",       result.Theme->Style.FrameRounding,       result.MissingFields);
            ParseStyleFloat(s, "ScrollbarSize",       result.Theme->Style.ScrollbarSize,       result.MissingFields);
            ParseStyleFloat(s, "ScrollbarRounding",   result.Theme->Style.ScrollbarRounding,   result.MissingFields);
            ParseStyleFloat(s, "GrabMinSize",         result.Theme->Style.GrabMinSize,         result.MissingFields);
            ParseStyleFloat(s, "GrabRounding",        result.Theme->Style.GrabRounding,        result.MissingFields);
            ParseStyleFloat(s, "TabRounding",         result.Theme->Style.TabRounding,         result.MissingFields);
            ParseStyleFloat(s, "TabBorderSize",       result.Theme->Style.TabBorderSize,       result.MissingFields);
            ParseStyleFloat(s, "IndentSpacing",       result.Theme->Style.IndentSpacing,       result.MissingFields);

            if (s.contains("FramePadding") && s["FramePadding"].is_array() && s["FramePadding"].size() >= 2)
            {
                result.Theme->Style.FramePaddingX = s["FramePadding"][0].get<float>();
                result.Theme->Style.FramePaddingY = s["FramePadding"][1].get<float>();
            }

            if (s.contains("ItemSpacing") && s["ItemSpacing"].is_array() && s["ItemSpacing"].size() >= 2)
            {
                result.Theme->Style.ItemSpacingX = s["ItemSpacing"][0].get<float>();
                result.Theme->Style.ItemSpacingY = s["ItemSpacing"][1].get<float>();
            }

            if (s.contains("ItemInnerSpacing") && s["ItemInnerSpacing"].is_array() && s["ItemInnerSpacing"].size() >= 2)
            {
                result.Theme->Style.ItemInnerSpacingX = s["ItemInnerSpacing"][0].get<float>();
                result.Theme->Style.ItemInnerSpacingY = s["ItemInnerSpacing"][1].get<float>();
            }
        }
    }
    catch (const nlohmann::json::exception& e)
    {
        result.ErrorMessage = e.what();
    }

    return result;
}

std::string ThemeSerializer::Serialize(const Theme& theme)
{
    nlohmann::json j;

    j["Name"] = theme.Name;

    j["Font"]["Regular"] = theme.FontRegular;
    j["Font"]["Bold"]    = theme.FontBold;
    j["Font"]["Size"]    = theme.FontSize;

    for (int i = 0; i < ImGuiCol_COUNT; ++i)
        SerializeColor(j["Colors"], s_ColorMap[i].Name, theme.Colors[i]);

    auto& s = j["Style"];
    s["WindowRounding"]    = theme.Style.WindowRounding;
    s["WindowBorderSize"]  = theme.Style.WindowBorderSize;
    s["ChildRounding"]     = theme.Style.ChildRounding;
    s["PopupRounding"]     = theme.Style.PopupRounding;
    s["FrameRounding"]     = theme.Style.FrameRounding;
    s["ScrollbarSize"]     = theme.Style.ScrollbarSize;
    s["ScrollbarRounding"] = theme.Style.ScrollbarRounding;
    s["GrabMinSize"]       = theme.Style.GrabMinSize;
    s["GrabRounding"]      = theme.Style.GrabRounding;
    s["TabRounding"]       = theme.Style.TabRounding;
    s["TabBorderSize"]     = theme.Style.TabBorderSize;
    s["IndentSpacing"]     = theme.Style.IndentSpacing;
    s["FramePadding"]      = { theme.Style.FramePaddingX, theme.Style.FramePaddingY };
    s["ItemSpacing"]       = { theme.Style.ItemSpacingX, theme.Style.ItemSpacingY };
    s["ItemInnerSpacing"]  = { theme.Style.ItemInnerSpacingX, theme.Style.ItemInnerSpacingY };

    return j.dump(4);
}

} // namespace AF
