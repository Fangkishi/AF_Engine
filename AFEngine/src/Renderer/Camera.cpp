#include "Renderer/Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace AF {

void Camera::SetPerspective(float fovDeg, float aspect, float nearPlane, float farPlane)
{
    m_Mode = Mode::Perspective;
    m_FOV = fovDeg;
    m_Aspect = aspect;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    RecalculateProjection();
}

void Camera::SetOrthographic(float size, float aspect, float nearPlane, float farPlane)
{
    m_Mode = Mode::Orthographic;
    m_OrthoSize = size;
    m_Aspect = aspect;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    RecalculateProjection();
}

void Camera::SetAspectRatio(float aspect)
{
    m_Aspect = aspect;
    RecalculateProjection();
}

void Camera::SetRotation(const glm::quat& rot)
{
    m_Rotation = rot;
    RecalculateView();
}

void Camera::RecalculateProjection()
{
    if (m_Mode == Mode::Perspective)
    {
        m_Projection = glm::perspective(glm::radians(m_FOV), m_Aspect, m_NearPlane, m_FarPlane);
    }
    else
    {
        float halfH = m_OrthoSize * 0.5f;
        float halfW = halfH * m_Aspect;
        m_Projection = glm::ortho(-halfW, halfW, -halfH, halfH, m_NearPlane, m_FarPlane);
    }
    m_ViewProjection = m_Projection * m_View;
}

void Camera::RecalculateView()
{
    glm::mat4 rot = glm::toMat4(m_Rotation);
    m_Front = glm::vec3(rot * glm::vec4(0, 0, -1, 1));
    m_View = glm::lookAt(m_Position, m_Position + m_Front, glm::vec3(0, 1, 0));
    m_ViewProjection = m_Projection * m_View;
}

} // namespace AF
