#include "Panels/HierarchyPanel.h"

#include <ECS/World.h>
#include <ECS/Components.h>

#include <imgui.h>
#include <algorithm>
#include <vector>

namespace AF {

HierarchyPanel::HierarchyPanel(World* world, CreateCallback onCreate, UUID& selectedUUID)
    : m_World(world)
    , m_OnCreate(std::move(onCreate))
    , m_SelectedUUID(selectedUUID)
{
}

void HierarchyPanel::OnImGuiRender()
{
    ImGui::Begin(GetName());

    if (!m_World)
    {
        ImGui::Text("(No world)");
        ImGui::End();
        return;
    }

    auto view = m_World->View<TagComponent, TransformComponent>();
    if (view.size_hint() == 0)
    {
        ImGui::Text("(No entities)");
    }

    // Sort by CreationOrder (oldest first, newest last — Unity-like)
    std::vector<std::pair<uint32_t, entt::entity>> sorted;
    for (auto [handle, tag, transform] : view.each())
    {
        Entity entity(handle, m_World);
        sorted.push_back({ entity.GetComponent<IDComponent>().CreationOrder, handle });
    }
    std::sort(sorted.begin(), sorted.end());

    bool anyClicked = false;
    bool anyEntityMenu = false;

    for (auto& [order, handle] : sorted)
    {
        Entity entity(handle, m_World);

        UUID uuid = entity.GetUUID();
        bool isSelected = (m_SelectedUUID == uuid);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_OpenOnArrow;

        if (isSelected)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool renamingThis = (m_RenamingUUID == uuid);

        if (renamingThis)
        {
            m_RenameBuffer.resize(128, '\0');
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputText("##rename", m_RenameBuffer.data(), m_RenameBuffer.size(),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                std::string newName(m_RenameBuffer.c_str());
                if (!newName.empty())
                    entity.GetComponent<TagComponent>().Tag = newName;
                m_RenamingUUID = UUID();
            }
            if (ImGui::IsItemDeactivated())
            {
                std::string newName(m_RenameBuffer.c_str());
                if (!newName.empty())
                    entity.GetComponent<TagComponent>().Tag = newName;
                m_RenamingUUID = UUID();
            }
        }
        else
        {
            bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(handle))),
                flags, "%s", entity.GetName().c_str());

            if (ImGui::IsItemClicked())
            {
                m_SelectedUUID = uuid;
                anyClicked = true;
            }

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
            {
                m_SelectedUUID = uuid;
                m_RenamingUUID = uuid;
                m_RenameBuffer = entity.GetName();
                m_RenameBuffer.resize(128, '\0');
            }

            if (ImGui::BeginPopupContextItem())
            {
                DrawEntityContextMenu(entity);
                anyEntityMenu = true;
                ImGui::EndPopup();
            }

            if (open)
                ImGui::TreePop();
        }
    }

    if (!anyEntityMenu && ImGui::BeginPopupContextWindow("HierarchyEmptyPopup",
        ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        DrawCreateMenu();
        ImGui::EndPopup();
    }

    if (anyClicked)
    {
        m_RenamingUUID = UUID();
    }

    if (m_SelectedUUID && !m_World->GetEntity(m_SelectedUUID))
        m_SelectedUUID = UUID();

    ImGui::End();
}

void HierarchyPanel::DrawCreateMenu()
{
    if (ImGui::BeginMenu("Create"))
    {
        if (ImGui::BeginMenu("3D Object"))
        {
            if (ImGui::MenuItem("Cube"))      m_OnCreate("Cube");
            if (ImGui::MenuItem("Plane"))     m_OnCreate("Plane");
            if (ImGui::MenuItem("Sphere"))    m_OnCreate("Sphere");
            if (ImGui::MenuItem("Cylinder"))  m_OnCreate("Cylinder");
            if (ImGui::MenuItem("Capsule"))   m_OnCreate("Capsule");
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
}

void HierarchyPanel::DrawEntityContextMenu(Entity entity)
{
    if (ImGui::MenuItem("Rename"))
    {
        m_SelectedUUID = entity.GetUUID();
        m_RenamingUUID = entity.GetUUID();
        m_RenameBuffer = entity.GetName();
        m_RenameBuffer.resize(128, '\0');
    }

    if (ImGui::MenuItem("Delete"))
    {
        if (m_SelectedUUID == entity.GetUUID())
            m_SelectedUUID = UUID();
        if (m_RenamingUUID == entity.GetUUID())
            m_RenamingUUID = UUID();
        m_World->DestroyEntity(entity);
    }
}

} // namespace AF
