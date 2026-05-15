#include "EditorCamera.h"

#include <Core/Input.h>
#include <Events/KeyCodes.h>
#include <Events/MouseCodes.h>

namespace AF {

glm::vec3 EditorCamera::GetForward() const
{
    float yawRad   = glm::radians(m_Yaw);
    float pitchRad = glm::radians(m_Pitch);
    return glm::vec3(
        cosf(yawRad) * cosf(pitchRad),
        sinf(pitchRad),
        sinf(yawRad) * cosf(pitchRad)
    );
}

glm::quat EditorCamera::GetRotation() const
{
    float yawRad   = glm::radians(m_Yaw);
    float pitchRad = glm::radians(m_Pitch);
    glm::quat qYaw   = glm::angleAxis(-(yawRad + glm::pi<float>() / 2.0f), glm::vec3(0, 1, 0));
    glm::quat qPitch = glm::angleAxis(pitchRad, glm::vec3(1, 0, 0));
    return qYaw * qPitch;
}

void EditorCamera::OnUpdate(float dt, Engine& engine)
{
    (void)engine;

    bool rightDown = Input::IsMouseButtonPressed(Mouse::ButtonRight);
    float mx = Input::GetMouseX();
    float my = Input::GetMouseY();

    if (rightDown)
    {
        if (m_FirstMouse)
        {
            m_LastMouseX = mx;
            m_LastMouseY = my;
            m_FirstMouse = false;
        }

        float dx = mx - m_LastMouseX;
        float dy = my - m_LastMouseY;
        m_LastMouseX = mx;
        m_LastMouseY = my;

        m_Yaw   += dx * m_RotateSpeed;
        m_Pitch -= dy * m_RotateSpeed;
        m_Pitch  = glm::clamp(m_Pitch, -89.0f, 89.0f);
    }
    else
    {
        m_FirstMouse = true;
    }

    float speed = m_MoveSpeed * dt;
    glm::vec3 forward = GetForward();
    glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (Input::IsKeyPressed(Key::W)) m_Position += forward * speed;
    if (Input::IsKeyPressed(Key::S)) m_Position -= forward * speed;
    if (Input::IsKeyPressed(Key::A)) m_Position -= right * speed;
    if (Input::IsKeyPressed(Key::D)) m_Position += right * speed;
    if (Input::IsKeyPressed(Key::Q)) m_Position.y -= speed;
    if (Input::IsKeyPressed(Key::E)) m_Position.y += speed;
}

} // namespace AF
