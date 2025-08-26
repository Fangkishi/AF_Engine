#include "afpch.h"
#include "AF/Renderer/Camera/OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace AF {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		:m_L(left), m_R(right), m_B(bottom), m_T(top)
	{
		AF_PROFILE_FUNCTION();

		m_ProjectionMatrix = glm::ortho(m_L, m_R, m_B, m_T, -1.0f, 1.0f);
		RecalculateViewMatrix();
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		AF_PROFILE_FUNCTION();

		m_L = left;
		m_R = right;
		m_B = bottom;
		m_T = top;
		m_Scale = 0.0f;

		m_ProjectionMatrix = glm::ortho(m_L, m_R, m_B, m_T, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		AF_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}