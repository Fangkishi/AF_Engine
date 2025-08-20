#include "afpch.h"
#include "AF/Renderer/Camera/OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace AF {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float n, float f)
		:m_L(left), m_R(right), m_B(bottom), m_T(top)
	{
		m_Near = n;
		m_Far = f;

		m_ProjectionMatrix = glm::ortho(m_L, m_R, m_B, m_T, m_Near, m_Far);
		RecalculateViewMatrix();
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top, float n, float f)
	{
		m_L = left;
		m_R = right;
		m_B = bottom;
		m_T = top;
		m_Near = n;
		m_Far = f;

		float scale = std::pow(2.0f, m_Scale);
		m_ProjectionMatrix = glm::ortho(m_L * scale, m_R * scale, m_B * scale, m_T * scale, m_Near, m_Far);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::scale(float deltaScale)
	{
		m_Scale -= deltaScale;

		float scale = std::pow(2.0f, m_Scale);
		m_ProjectionMatrix = glm::ortho(m_L * scale, m_R * scale, m_B * scale, m_T * scale, m_Near, m_Far);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}