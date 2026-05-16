#pragma once

#include "Panels/Panel.h"
#include <RHI/RHITexture.h>

#include <functional>
#include <string>
#include <vector>

namespace AF {

class ViewportPanel : public Panel
{
public:
    const char* GetName() const override { return "Viewport"; }
    void OnImGuiRender() override;

    uint32_t GetContentWidth()  const { return m_ContentWidth;  }
    uint32_t GetContentHeight() const { return m_ContentHeight; }
    void SetRenderTexture(Ref<RHI::RHITexture2D> tex) { m_RenderTexture = std::move(tex); }

    bool IsHovered() const { return m_Hovered; }

    void SetCameraList(const std::vector<std::string>& names, int activeIndex,
                       std::function<void(int)> onSelect);

private:
    uint32_t m_ContentWidth  = 0;
    uint32_t m_ContentHeight = 0;
    Ref<RHI::RHITexture2D> m_RenderTexture;
    bool m_Hovered = false;

    std::vector<std::string> m_CameraNames;
    int m_ActiveCameraIndex = 0;
    std::function<void(int)> m_OnCameraSelect;
};

} // namespace AF
