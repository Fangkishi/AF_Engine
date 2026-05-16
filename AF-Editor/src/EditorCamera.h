#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Renderer/Camera.h>

namespace AF {

class Engine;

class EditorCamera
{
public:
    EditorCamera() = default;

    void OnUpdate(float dt, Engine& engine);

    glm::vec3 GetPosition() const { return m_Position; }
    glm::vec3 GetForward()  const;
    glm::quat GetRotation() const;

    const glm::mat4& GetProjection() const { return m_Camera.GetProjection(); }
    glm::mat4 GetView() const;
    const glm::vec3& GetCameraForward() const { return m_Camera.GetForward(); }

    Camera& GetCamera() { return m_Camera; }

private:
    void UpdateView();

    Camera m_Camera;

    glm::vec3 m_Position   = { 0.0f, 1.0f, 5.0f };
    float m_Yaw   = -90.0f;
    float m_Pitch = 0.0f;
    float m_MoveSpeed   = 5.0f;
    float m_RotateSpeed = 0.2f;
    float m_LastMouseX  = 0.0f;
    float m_LastMouseY  = 0.0f;
    bool  m_FirstMouse  = true;
};

} // namespace AF
