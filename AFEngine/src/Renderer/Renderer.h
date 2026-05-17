#pragma once

// RenderSystem —— 渲染系统
//
// 核心职责：
// 1. 每帧收集 ECS 世界的相机、Mesh、光源数据 → m_View + m_Packet
// 2. 支持编辑器覆盖相机（m_CameraOverride）
// 3. 将实体按材质混合模式分为 Opaque 和 Translucent 包
//
// 它是 System 子类，直接注册到 Engine，不继承 RenderPipeline。

#include "Core/System.h"
#include "Core/Types.h"
#include "Renderer/RenderView.h"
#include "Renderer/RenderPacket.h"
#include "ECS/Components.h"

namespace AF {

class RenderSystem : public System
{
public:
    static RenderSystem* Instance;
    static RenderSystem& Get() { return *Instance; }

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
