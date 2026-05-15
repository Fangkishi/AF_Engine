#include "UI/Theme.h"

namespace AF {

Theme Theme::CreateDefaultDark()
{
    Theme t;
    t.Name        = "Built-in Dark";
    t.FontRegular = "Resources/Fonts/opensans/OpenSans-Regular.ttf";
    t.FontBold    = "Resources/Fonts/opensans/OpenSans-Bold.ttf";
    t.FontSize    = 16.0f;

    t.Colors[ImGuiCol_Text]                  = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    t.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    t.Colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    t.Colors[ImGuiCol_ChildBg]               = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    t.Colors[ImGuiCol_PopupBg]               = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    t.Colors[ImGuiCol_Border]                = ImVec4(0.22f, 0.22f, 0.23f, 1.00f);
    t.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    t.Colors[ImGuiCol_FrameBg]               = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    t.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
    t.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
    t.Colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    t.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    t.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    t.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
    t.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.06f, 0.06f, 0.07f, 0.53f);
    t.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.22f, 0.22f, 0.23f, 1.00f);
    t.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.28f, 0.28f, 0.29f, 1.00f);
    t.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.34f, 0.34f, 0.35f, 1.00f);
    t.Colors[ImGuiCol_CheckMark]             = ImVec4(0.80f, 0.80f, 0.81f, 1.00f);
    t.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
    t.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.36f, 0.36f, 0.37f, 1.00f);
    t.Colors[ImGuiCol_Button]                = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    t.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.22f, 0.22f, 0.23f, 1.00f);
    t.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    t.Colors[ImGuiCol_Header]                = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    t.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
    t.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
    t.Colors[ImGuiCol_Separator]             = ImVec4(0.22f, 0.22f, 0.23f, 1.00f);
    t.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.32f, 0.32f, 0.33f, 1.00f);
    t.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.40f, 0.40f, 0.41f, 1.00f);
    t.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    t.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
    t.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.36f, 0.36f, 0.37f, 1.00f);
    t.Colors[ImGuiCol_Tab]                   = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    t.Colors[ImGuiCol_TabHovered]            = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    t.Colors[ImGuiCol_TabActive]             = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    t.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    t.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    t.Colors[ImGuiCol_DockingPreview]        = ImVec4(0.34f, 0.34f, 0.35f, 0.70f);
    t.Colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    t.Colors[ImGuiCol_PlotLines]             = ImVec4(0.60f, 0.60f, 0.61f, 1.00f);
    t.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.80f, 0.80f, 0.81f, 1.00f);
    t.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.55f, 0.55f, 0.56f, 1.00f);
    t.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.70f, 0.70f, 0.71f, 1.00f);
    t.Colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    t.Colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.22f, 0.22f, 0.23f, 1.00f);
    t.Colors[ImGuiCol_TableBorderLight]      = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
    t.Colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    t.Colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.06f, 0.06f, 0.07f, 0.06f);
    t.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.34f, 0.34f, 0.35f, 0.50f);
    t.Colors[ImGuiCol_DragDropTarget]        = ImVec4(0.40f, 0.40f, 0.41f, 1.00f);
    t.Colors[ImGuiCol_NavHighlight]          = ImVec4(0.28f, 0.28f, 0.29f, 1.00f);
    t.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.60f, 0.60f, 0.61f, 0.70f);
    t.Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.40f, 0.40f, 0.41f, 0.20f);
    t.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.40f, 0.40f, 0.41f, 0.25f);

    return t;
}

Theme Theme::CreateDefaultLight()
{
    Theme t = CreateDefaultDark();
    t.Name = "Built-in Light";
    return t;
}

} // namespace AF
