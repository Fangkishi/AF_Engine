#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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

private:
    glm::vec3 m_Position   = { 0.0f, 0.0f, 5.0f };
    float m_Yaw   = -90.0f;
    float m_Pitch = 0.0f;
    float m_MoveSpeed   = 5.0f;
    float m_RotateSpeed = 0.2f;
    float m_LastMouseX  = 0.0f;
    float m_LastMouseY  = 0.0f;
    bool  m_FirstMouse  = true;
};

} // namespace AF
