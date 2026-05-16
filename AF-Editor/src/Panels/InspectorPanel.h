#pragma once

#include "Panels/Panel.h"

#include <Core/UUID.h>
#include <ECS/Entity.h>

namespace AF {

class World;

class InspectorPanel : public Panel
{
public:
    InspectorPanel(World* world, const UUID& selectedUUID);

    const char* GetName() const override { return "Inspector"; }
    void OnImGuiRender() override;

private:
    void DrawHeader(Entity entity);
    void DrawTransformComponent(Entity entity);
    void DrawMeshComponent(Entity entity);
    void DrawMaterialComponent(Entity entity);
    void DrawLightComponent(Entity entity);
    void DrawCameraComponent(Entity entity);

    World* m_World;
    const UUID& m_SelectedUUID;

    float m_CachedEuler[3] = { 0, 0, 0 };
    UUID m_CachedEntityUUID;
    bool m_ScaleLocked = false;
};

} // namespace AF
