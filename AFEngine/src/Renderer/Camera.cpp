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

void Camera::SetFOV(float fovDeg)
{
    if (m_FOV == fovDeg) return;
    m_FOV = fovDeg;
    RecalculateProjection();
}

void Camera::SetNearPlane(float nearPlane)
{
    if (m_NearPlane == nearPlane) return;
    m_NearPlane = nearPlane;
    RecalculateProjection();
}

void Camera::SetFarPlane(float farPlane)
{
    if (m_FarPlane == farPlane) return;
    m_FarPlane = farPlane;
    RecalculateProjection();
}

void Camera::SetMode(Mode mode)
{
    if (m_Mode == mode) return;
    m_Mode = mode;
    RecalculateProjection();
}

/// 设置旋转四元数并重新计算 View 矩阵
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

/// 从四元数计算 view 方向和 up，构建 View 矩阵
void Camera::RecalculateView()
{
    glm::mat4 rot = glm::toMat4(m_Rotation);
    m_Front = glm::vec3(rot * glm::vec4(0, 0, -1, 1));
    glm::vec3 up = m_Rotation * glm::vec3(0, 1, 0);
    m_View = glm::lookAt(m_Position, m_Position + m_Front, up);
    m_ViewProjection = m_Projection * m_View;
}

} // namespace AF
