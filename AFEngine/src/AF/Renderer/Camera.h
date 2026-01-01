#pragma once

#include <glm/glm.hpp>

namespace AF {
	class Camera
	{
	public:
		Camera() = default;

		Camera(const glm::mat4& projection)
			: m_Projection(projection)
		{
		}

		virtual ~Camera() = default;

		void SetPosition(glm::vec3& position) { m_Position = position; }
		void SetViewMatrix(glm::mat4& viewMatrix) { m_ViewMatrix = viewMatrix; }

		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjection() const { return m_Projection; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		virtual float GetPerspectiveNearClip() const = 0;

		virtual float GetPerspectiveFarClip() const = 0;
	protected:
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::mat4 m_Projection = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
	};
}
