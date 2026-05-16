#pragma once

#include "Panels/Panel.h"

#include <Core/UUID.h>
#include <ECS/Entity.h>

#include <functional>
#include <string>

namespace AF {

class World;

class HierarchyPanel : public Panel
{
public:
    using CreateCallback = std::function<void(const std::string& type)>;

    HierarchyPanel(World* world, CreateCallback onCreate, UUID& selectedUUID);

    const char* GetName() const override { return "Scene Hierarchy"; }
    void OnImGuiRender() override;

private:
    void DrawCreateMenu();
    void DrawEntityContextMenu(Entity entity);

    World* m_World = nullptr;
    CreateCallback m_OnCreate;
    UUID& m_SelectedUUID;
    UUID m_RenamingUUID;
    std::string m_RenameBuffer;
};

} // namespace AF
