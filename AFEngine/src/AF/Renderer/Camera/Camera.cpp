#include "afpch.h"
#include "camera.h"

namespace AF {

	Camera::Camera()
	{

	}

	Camera::~Camera()
	{

	}

	void Camera::RecalculateViewMatrix()
	{
		AF_PROFILE_FUNCTION();

		glm::vec3 front = glm::cross(m_Up, m_Right);
		glm::vec3 center = m_Position + front;

		m_ViewMatrix = glm::lookAt(m_Position, center, m_Up);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}

