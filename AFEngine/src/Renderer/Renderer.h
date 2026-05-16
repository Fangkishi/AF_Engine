#pragma once

#include "Core/System.h"
#include "Core/Types.h"
#include "Renderer/RenderView.h"
#include "Renderer/RenderPacket.h"
#include "ECS/Components.h"

namespace AF {

class RenderSystem : public System
{
public:
    void OnInitialize(Engine& engine) override;
    void OnUpdate(float dt) override;
    void OnEvent(Event& event) override;

    void SetViewport(uint32_t width, uint32_t height);
    void SetCameraView(const RenderView& view);

    const RenderView&   GetView()   const { return m_View;   }
    const RenderPacket& GetPacket() const { return m_Packet; }

private:
    RenderView   m_View;
    RenderPacket m_Packet;
    uint32_t m_ViewportWidth  = 1920;
    uint32_t m_ViewportHeight = 1080;
    bool m_CameraOverride = false;
};

} // namespace AF
