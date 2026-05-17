#include "UI/ImGuiThemeApplier.h"

#include "Core/Log.h"

#include <imgui.h>
#include <filesystem>

namespace AF {

void ImGuiThemeApplier::ApplyColors(const ImVec4 colors[ImGuiCol_COUNT])
{
    ImGuiStyle& dst = ImGui::GetStyle();
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
        dst.Colors[i] = colors[i];
}

void ImGuiThemeApplier::ApplyStyle(const ThemeStyle& s)
{
    ImGuiStyle& dst = ImGui::GetStyle();
    dst.WindowRounding      = s.WindowRounding;
    dst.WindowBorderSize    = s.WindowBorderSize;
    dst.ChildRounding       = s.ChildRounding;
    dst.ChildBorderSize     = 0.0f;
    dst.PopupRounding       = s.PopupRounding;
    dst.PopupBorderSize     = 0.0f;
    dst.FramePadding        = ImVec2(s.FramePaddingX, s.FramePaddingY);
    dst.FrameRounding       = s.FrameRounding;
    dst.FrameBorderSize     = 0.0f;
    dst.ItemSpacing         = ImVec2(s.ItemSpacingX, s.ItemSpacingY);
    dst.ItemInnerSpacing    = ImVec2(s.ItemInnerSpacingX, s.ItemInnerSpacingY);
    dst.IndentSpacing       = s.IndentSpacing;
    dst.ScrollbarSize       = s.ScrollbarSize;
    dst.ScrollbarRounding   = s.ScrollbarRounding;
    dst.GrabMinSize         = s.GrabMinSize;
    dst.GrabRounding        = s.GrabRounding;
    dst.TabRounding         = s.TabRounding;
    dst.TabBorderSize       = s.TabBorderSize;
}

bool ImGuiThemeApplier::LoadFont(const std::string& regularPath,
                                 const std::string& boldPath, float size)
{
    ImGuiIO& io = ImGui::GetIO();
    std::error_code ec;

    // 尝试加载粗体字体
    if (!boldPath.empty() && std::filesystem::exists(boldPath, ec))
    {
        ImFont* boldFont = io.Fonts->AddFontFromFileTTF(boldPath.c_str(), size);
        if (!boldFont)
            AF_LOG_WARN("ImGuiThemeApplier: failed to load bold font '{}'", boldPath);
    }
    else if (!boldPath.empty())
    {
        AF_LOG_WARN("ImGuiThemeApplier: bold font file not found '{}'", boldPath);
    }

    // 加载常规字体（必需）
    if (!regularPath.empty() && std::filesystem::exists(regularPath, ec))
    {
        ImFont* regularFont = io.Fonts->AddFontFromFileTTF(regularPath.c_str(), size);
        if (!regularFont)
        {
            AF_LOG_ERROR("ImGuiThemeApplier: failed to load regular font '{}'", regularPath);
            return false;
        }
        io.FontDefault = regularFont;
        return true;
    }

    AF_LOG_WARN("ImGuiThemeApplier: regular font file not found '{}'", regularPath);
    return false;
}

} // namespace AF
