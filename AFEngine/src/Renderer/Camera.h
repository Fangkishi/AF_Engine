#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace AF {

class Camera
{
public:
    enum class Mode { Perspective, Orthographic };

    Camera() = default;

    void SetPerspective(float fovDeg, float aspect, float nearPlane, float farPlane);
    void SetOrthographic(float size, float aspect, float nearPlane, float farPlane);

    void SetPosition(const glm::vec3& pos) { m_Position = pos; }
    void SetRotation(const glm::quat& rot);

    void SetAspectRatio(float aspect);

    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetForward()  const { return m_Front;  }

    const glm::mat4& GetProjection()     const { return m_Projection; }
    const glm::mat4& GetView()           const { return m_View;       }
    const glm::mat4& GetViewProjection() const { return m_ViewProjection; }

    Mode  GetMode()       const { return m_Mode;       }
    float GetFOV()        const { return m_FOV;        }
    float GetNearPlane()  const { return m_NearPlane;  }
    float GetFarPlane()   const { return m_FarPlane;   }
    float GetOrthoSize()  const { return m_OrthoSize;  }

private:
    void RecalculateProjection();
    void RecalculateView();

    glm::mat4 m_Projection      = glm::mat4(1.0f);
    glm::mat4 m_View            = glm::mat4(1.0f);
    glm::mat4 m_ViewProjection  = glm::mat4(1.0f);

    glm::vec3 m_Position = { 0.0f, 0.0f, 3.0f };
    glm::quat m_Rotation = glm::identity<glm::quat>();
    glm::vec3 m_Front    = { 0.0f, 0.0f, -1.0f };

    Mode m_Mode = Mode::Perspective;
    float m_FOV        = 60.0f;
    float m_Aspect     = 1.78f;
    float m_NearPlane  = 0.1f;
    float m_FarPlane   = 100.0f;
    float m_OrthoSize  = 10.0f;
};

} // namespace AF
