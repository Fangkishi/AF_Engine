#include "afpch.h"
#include "AF/Renderer/Camera/OrthographicCamera2D.h"

#include <glm/gtc/matrix_transform.hpp>

namespace AF {

	OrthographicCamera2D::OrthographicCamera2D(float left, float right, float bottom, float top)
		:m_L(left), m_R(right), m_B(bottom), m_T(top)
	{
		m_ProjectionMatrix = glm::ortho(m_L, m_R, m_B, m_T, -1.0f, 1.0f);
		RecalculateViewMatrix();
	}

	void OrthographicCamera2D::SetProjection(float left, float right, float bottom, float top)
	{
		m_L = left;
		m_R = right;
		m_B = bottom;
		m_T = top;

		float scale = std::pow(2.0f, m_Scale);
		m_ProjectionMatrix = glm::ortho(m_L * scale, m_R * scale, m_B * scale, m_T * scale, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera2D::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera2D::scale(float deltaScale)
	{
		m_Scale -= deltaScale;

		float scale = std::pow(2.0f, m_Scale);
		m_ProjectionMatrix = glm::ortho(m_L * scale, m_R * scale, m_B * scale, m_T * scale, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}