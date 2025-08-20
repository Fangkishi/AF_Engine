#include "afpch.h"
#include "PerspectiveCamera.h"

namespace AF {

	PerspectiveCamera::PerspectiveCamera(float fovy, float aspect, float n, float f)
		:m_Fovy(fovy), m_Aspect(aspect)
	{
		m_Near = n;
		m_Far = f;

		m_ProjectionMatrix = glm::perspective(glm::radians(m_Fovy), m_Aspect, m_Near, m_Far);
		RecalculateViewMatrix();
	}

	void PerspectiveCamera::SetProjection(float fovy, float aspect, float n, float f)
	{
		m_Fovy = fovy;
		m_Aspect = aspect;
		m_Near = n;
		m_Far = f;

		m_ProjectionMatrix = glm::perspective(glm::radians(m_Fovy), m_Aspect, m_Near, m_Far);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::scale(float deltaScale) {
		auto front = glm::normalize(glm::cross(m_Up, m_Right));
		glm::vec3 position = m_Position += (front * deltaScale);
		SetPosition(position);
	}

}