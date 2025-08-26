#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace AF {

	class Camera {
	public:
		Camera() = default;
		virtual ~Camera() = default;

		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		virtual void SetProjection(float fovy, float aspect, float n, float f) = 0;

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		virtual void RecalculateViewMatrix() = 0;
	protected:
		float m_ZoomLevel = 1.0f;

		glm::mat4 m_ProjectionMatrix{ 1.0 };
		glm::mat4 m_ViewMatrix{ 1.0 };
		glm::mat4 m_ViewProjectionMatrix{ 1.0 };

		glm::vec3 m_Position{ 0.0f, 0.0f, 0.0f };
	};

}