#include "Panels/InspectorPanel.h"

#include <ECS/World.h>
#include <ECS/Components.h>
#include <ECS/Entity.h>
#include <Renderer/Camera.h>
#include <Renderer/Mesh.h>
#include <Renderer/Material.h>
#include <Factory/MeshFactory.h>
#include <Factory/MaterialFactory.h>

#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <memory>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace AF {

namespace {

const float kEpsilon = 0.001f;
inline float ClampZero(float v) { return (fabsf(v) < kEpsilon) ? 0.0f : v; }

// ── Euler → Quaternion ──────────────────────────────────

glm::quat EulerDegToQuatLocal(const glm::vec3& deg)
{
    glm::vec3 r = glm::radians(deg);
    return glm::angleAxis(r.y, glm::vec3(0, 1, 0))
         * glm::angleAxis(r.x, glm::vec3(1, 0, 0))
         * glm::angleAxis(r.z, glm::vec3(0, 0, 1));
}

glm::quat EulerDegToQuatWorld(const glm::vec3& deg)
{
    glm::vec3 r = glm::radians(deg);
    return glm::angleAxis(r.z, glm::vec3(0, 0, 1))
         * glm::angleAxis(r.y, glm::vec3(0, 1, 0))
         * glm::angleAxis(r.x, glm::vec3(1, 0, 0));
}

// ── Quaternion → Euler ──────────────────────────────────

glm::vec3 QuatToEulerDegLocal(const glm::quat& q)
{
    glm::mat3 m = glm::toMat3(q);
    float sinP = -m[1][2];

    float pitch, yaw, roll;
    if (fabsf(sinP) >= 0.99999f)
    {
        float sign = (sinP > 0.0f) ? 1.0f : -1.0f;
        pitch = sign * glm::half_pi<float>();
        yaw   = 0.0f;
        roll  = sign * atan2f(-m[2][0], m[0][0]);
    }
    else
    {
        pitch = asinf(sinP);
        yaw   = atan2f(m[0][2], m[2][2]);
        roll  = atan2f(m[1][0], m[1][1]);
    }
    return glm::vec3(ClampZero(glm::degrees(pitch)),
                     ClampZero(glm::degrees(yaw)),
                     ClampZero(glm::degrees(roll)));
}

glm::vec3 QuatToEulerDegWorld(const glm::quat& q)
{
    glm::mat3 m = glm::toMat3(q);
    float sinY = -m[2][0];

    float pitch, yaw, roll;
    if (fabsf(sinY) >= 0.99999f)
    {
        float sign = (sinY > 0.0f) ? 1.0f : -1.0f;
        yaw   = sign * glm::half_pi<float>();
        pitch = 0.0f;
        if (sign > 0.0f)
            roll = -atan2f(m[1][1], -m[1][2]);
        else
            roll =  atan2f(m[1][1], m[1][2]);
    }
    else
    {
        yaw   = asinf(sinY);
        pitch = atan2f(m[2][1], m[2][2]);
        roll  = atan2f(m[1][0], m[0][0]);
    }
    return glm::vec3(ClampZero(glm::degrees(pitch)),
                     ClampZero(glm::degrees(yaw)),
                     ClampZero(glm::degrees(roll)));
}

// ── Axis drawing helpers ────────────────────────────────

void DrawColoredAxis(const char* id, const char* label, const ImVec4& bgColor, float* v, float speed, float min, float max, float resetValue)
{
    float lineHeight = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    ImVec4 hoverColor(bgColor.x * 1.15f, bgColor.y * 1.15f, bgColor.z * 1.15f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushID(id);
    if (ImGui::Button(label, buttonSize))
        *v = resetValue;
    ImGui::PopID();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::SameLine(0, 0);
    ImGui::SetNextItemWidth(60);
    ImGui::DragFloat(id, v, speed, min, max, "%.2f");
}

void DrawTransformLine(const char* propLabel, float* x, float* y, float* z, float speed, float min, float max, float resetValue)
{
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(propLabel);
    ImGui::SameLine(70);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    std::string base = std::string("##") + propLabel;
    DrawColoredAxis((base + "X").c_str(), "X", ImVec4(0.8f, 0.1f, 0.15f, 1.0f), x, speed, min, max, resetValue);
    ImGui::SameLine(0, 3);
    DrawColoredAxis((base + "Y").c_str(), "Y", ImVec4(0.2f, 0.7f, 0.2f, 1.0f), y, speed, min, max, resetValue);
    ImGui::SameLine(0, 3);
    DrawColoredAxis((base + "Z").c_str(), "Z", ImVec4(0.1f, 0.25f, 0.8f, 1.0f), z, speed, min, max, resetValue);

    ImGui::PopStyleVar();
}

template <typename T>
bool DrawComponentHeader(const char* name, const ImVec4& color, Entity& entity)
{
    ImVec4 hover  = ImVec4(color.x * 1.3f, color.y * 1.3f, color.z * 1.3f, 1.0f);
    ImVec4 active = ImVec4(color.x * 1.15f, color.y * 1.15f, color.z * 1.15f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Header, color);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hover);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, active);

    bool opened = ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen);

    ImGui::PopStyleColor(3);

    if (ImGui::BeginPopupContextItem(name))
    {
        if (ImGui::MenuItem("Remove Component"))
        {
            entity.RemoveComponent<T>();
            ImGui::EndPopup();
            return false;
        }
        ImGui::EndPopup();
    }

    return opened;
}

} // anonymous namespace

InspectorPanel::InspectorPanel(World* world, const UUID& selectedUUID)
    : m_World(world)
    , m_SelectedUUID(selectedUUID)
{
}

void InspectorPanel::OnImGuiRender()
{
    ImGui::Begin(GetName());

    if (!m_World)
    {
        ImGui::Text("(No world)");
        ImGui::End();
        return;
    }

    if (!m_SelectedUUID)
    {
        ImGui::Text("No entity selected");
        ImGui::End();
        return;
    }

    Entity entity = m_World->GetEntity(m_SelectedUUID);
    if (!entity)
    {
        ImGui::Text("Selected entity no longer exists");
        ImGui::End();
        return;
    }

    DrawHeader(entity);

    ImGui::Separator();

    ImGui::PushItemWidth(-1);

    DrawTransformComponent(entity);

    if (entity.HasComponent<MeshComponent>())
    {
        ImGui::Spacing();
        DrawMeshComponent(entity);
    }

    if (entity.HasComponent<MaterialComponent>())
    {
        ImGui::Spacing();
        DrawMaterialComponent(entity);
    }

    if (entity.HasComponent<LightComponent>())
    {
        ImGui::Spacing();
        DrawLightComponent(entity);
    }

    if (entity.HasComponent<CameraComponent>())
    {
        ImGui::Spacing();
        DrawCameraComponent(entity);
    }

    ImGui::PopItemWidth();
    ImGui::End();
}

void InspectorPanel::DrawHeader(Entity entity)
{
    auto& tag = entity.GetComponent<TagComponent>();

    char buf[256];
    strncpy_s(buf, sizeof(buf), tag.Tag.c_str(), _TRUNCATE);

    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 130.0f);
    if (ImGui::InputText("##EntityName", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
        tag.Tag = buf;
    ImGui::PopItemWidth();

    ImGui::SameLine();

    bool hasMesh     = entity.HasComponent<MeshComponent>();
    bool hasMaterial = entity.HasComponent<MaterialComponent>();
    bool hasLight    = entity.HasComponent<LightComponent>();
    bool hasCamera   = entity.HasComponent<CameraComponent>();

    if (ImGui::Button("Add Component"))
        ImGui::OpenPopup("AddComponentPopup");

    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        if (!hasMesh && ImGui::MenuItem("Static Mesh"))
        {
            entity.AddComponent<MeshComponent>(MeshFactory::CreateCube());
            ImGui::CloseCurrentPopup();
        }
        if (!hasMaterial && ImGui::MenuItem("Material"))
        {
            entity.AddComponent<MaterialComponent>(MaterialFactory::CreateDefault());
            ImGui::CloseCurrentPopup();
        }
        if (!hasLight && ImGui::MenuItem("Light"))
        {
            entity.AddComponent<LightComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (!hasCamera && ImGui::MenuItem("Camera"))
        {
            auto camera = std::make_shared<Camera>();
            camera->SetPerspective(60.0f, 1.78f, 0.1f, 100.0f);
            entity.AddComponent<CameraComponent>(camera);
            ImGui::CloseCurrentPopup();
        }
        if (hasMesh && hasMaterial && hasLight && hasCamera)
            ImGui::TextDisabled("(All components added)");
        ImGui::EndPopup();
    }
}

void InspectorPanel::DrawTransformComponent(Entity entity)
{
    ImVec4 color(0.30f, 0.35f, 0.42f, 1.0f);
    ImVec4 hover(color.x * 1.3f, color.y * 1.3f, color.z * 1.3f, 1.0f);
    ImVec4 active(color.x * 1.15f, color.y * 1.15f, color.z * 1.15f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Header, color);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hover);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, active);

    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PopStyleColor(3);
        return;
    }
    ImGui::PopStyleColor(3);

    auto& transform = entity.GetComponent<TransformComponent>();

    // ── Position ─────────────────────────────────────────

    float pos[3] = { transform.Position.x, transform.Position.y, transform.Position.z };
    DrawTransformLine("Position", &pos[0], &pos[1], &pos[2], 0.1f, 0.0f, 0.0f, 0.0f);
    if (memcmp(pos, &transform.Position, sizeof(pos)) != 0)
        transform.Position = { pos[0], pos[1], pos[2] };

    // ── Rotation ─────────────────────────────────────────

    if (m_CachedEntityUUID != entity.GetUUID())
    {
        m_CachedEntityUUID = entity.GetUUID();
        glm::vec3 euler = QuatToEulerDegLocal(transform.Rotation);
        m_CachedEuler[0] = euler.x;
        m_CachedEuler[1] = euler.y;
        m_CachedEuler[2] = euler.z;
    }

    float oldRot[3] = { m_CachedEuler[0], m_CachedEuler[1], m_CachedEuler[2] };
    DrawTransformLine("Rotation", &m_CachedEuler[0], &m_CachedEuler[1], &m_CachedEuler[2], 0.5f, 0.0f, 0.0f, 0.0f);
    if (memcmp(m_CachedEuler, oldRot, sizeof(m_CachedEuler)) != 0)
        transform.Rotation = EulerDegToQuatLocal({ m_CachedEuler[0], m_CachedEuler[1], m_CachedEuler[2] });

    // ── Scale ────────────────────────────────────────────

    float scl[3] = { transform.Scale.x, transform.Scale.y, transform.Scale.z };
    float oldScl[3] = { scl[0], scl[1], scl[2] };

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Scale");
    ImGui::SameLine(70);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    std::string base = "##Scale";
    DrawColoredAxis((base + "X").c_str(), "X", ImVec4(0.8f, 0.1f, 0.15f, 1.0f), &scl[0], 0.1f, 0.01f, 100.0f, 1.0f);
    ImGui::SameLine(0, 3);
    DrawColoredAxis((base + "Y").c_str(), "Y", ImVec4(0.2f, 0.7f, 0.2f, 1.0f), &scl[1], 0.1f, 0.01f, 100.0f, 1.0f);
    ImGui::SameLine(0, 3);
    DrawColoredAxis((base + "Z").c_str(), "Z", ImVec4(0.1f, 0.25f, 0.8f, 1.0f), &scl[2], 0.1f, 0.01f, 100.0f, 1.0f);

    ImGui::PopStyleVar();

    ImGui::SameLine(0, 4);
    ImGui::Checkbox("##ScaleLock", &m_ScaleLocked);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(m_ScaleLocked ? "Proportional scale locked" : "Proportional scale unlocked");

    if (m_ScaleLocked)
    {
        for (int i = 0; i < 3; i++)
        {
            float delta = scl[i] - oldScl[i];
            if (fabsf(delta) <= kEpsilon)
                continue;

            if (fabsf(oldScl[i]) < kEpsilon)
            {
                float v = scl[i];
                scl[0] = scl[1] = scl[2] = v;
            }
            else
            {
                float ratio = scl[i] / oldScl[i];
                scl[0] = oldScl[0] * ratio;
                scl[1] = oldScl[1] * ratio;
                scl[2] = oldScl[2] * ratio;
            }
            break;
        }
    }

    if (memcmp(scl, oldScl, sizeof(scl)) != 0)
        transform.Scale = { scl[0], scl[1], scl[2] };
}

void InspectorPanel::DrawMeshComponent(Entity entity)
{
    if (!DrawComponentHeader<MeshComponent>("Static Mesh", ImVec4(0.28f, 0.33f, 0.40f, 1.0f), entity))
        return;

    auto& meshComp = entity.GetComponent<MeshComponent>();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Vertices");
    ImGui::SameLine(80);
    ImGui::TextDisabled("%u", meshComp.Source ? meshComp.Source->GetIndexCount() / 3 * 3 : 0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Triangles");
    ImGui::SameLine(80);
    ImGui::TextDisabled("%u", meshComp.Source ? meshComp.Source->GetIndexCount() / 3 : 0);
}

void InspectorPanel::DrawMaterialComponent(Entity entity)
{
    if (!DrawComponentHeader<MaterialComponent>("Material", ImVec4(0.28f, 0.33f, 0.40f, 1.0f), entity))
        return;

    auto& matComp = entity.GetComponent<MaterialComponent>();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Material");
    ImGui::SameLine(80);
    ImGui::TextDisabled("%s", matComp.Source ? "Default" : "None");
}

void InspectorPanel::DrawLightComponent(Entity entity)
{
    if (!DrawComponentHeader<LightComponent>("Light", ImVec4(0.35f, 0.30f, 0.25f, 1.0f), entity))
        return;

    auto& light = entity.GetComponent<LightComponent>();

    float col[3] = { light.Color.r, light.Color.g, light.Color.b };
    if (ImGui::ColorEdit3("Color", col))
        light.Color = { col[0], col[1], col[2] };

    ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 100.0f);

    const char* types[] = { "Directional", "Point" };
    const char* preview = (light.Type < 2) ? types[light.Type] : "Unknown";
    if (ImGui::BeginCombo("Type", preview))
    {
        for (int i = 0; i < 2; i++)
        {
            bool selected = (static_cast<uint32_t>(i) == light.Type);
            if (ImGui::Selectable(types[i], selected))
                light.Type = static_cast<uint32_t>(i);
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void InspectorPanel::DrawCameraComponent(Entity entity)
{
    if (!DrawComponentHeader<CameraComponent>("Camera", ImVec4(0.25f, 0.30f, 0.35f, 1.0f), entity))
        return;

    auto& camComp = entity.GetComponent<CameraComponent>();
    if (!camComp.Source)
    {
        ImGui::TextDisabled("(No Camera reference)");
        return;
    }

    auto& camera = *camComp.Source;

    float fov = camera.GetFOV();
    if (ImGui::DragFloat("FOV", &fov, 0.5f, 1.0f, 179.0f))
        camera.SetFOV(fov);

    float nearP = camera.GetNearPlane();
    if (ImGui::DragFloat("Near", &nearP, 0.01f, 0.001f, 1000.0f, "%.3f"))
        camera.SetNearPlane(nearP);

    float farP = camera.GetFarPlane();
    if (ImGui::DragFloat("Far", &farP, 1.0f, 0.1f, 10000.0f, "%.1f"))
        camera.SetFarPlane(farP);

    bool primary = camComp.Primary;
    if (ImGui::Checkbox("Primary", &primary))
        camComp.Primary = primary;

    const char* modes[] = { "Perspective", "Orthographic" };
    int modeIdx = (camera.GetMode() == Camera::Mode::Perspective) ? 0 : 1;
    const char* preview = modes[modeIdx];
    if (ImGui::BeginCombo("Mode", preview))
    {
        for (int i = 0; i < 2; i++)
        {
            bool selected = (i == modeIdx);
            if (ImGui::Selectable(modes[i], selected))
                camera.SetMode((i == 0) ? Camera::Mode::Perspective : Camera::Mode::Orthographic);
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

} // namespace AF
