#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace AF {
	class CameraBase
	{
	public:
		CameraBase() = default;

		CameraBase(const glm::mat4& projection)
			: m_ProjectionMatrix(projection)
		{
		}

		virtual ~CameraBase() = default;

		const glm::vec3& GetPosition() const { return m_Position; }

		void SetPosition(const glm::vec3& position)
		{
			m_Position = position;
			RecalculateViewMatrix();
		}

		const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
		virtual void SetProjection(float fovy, float aspect, float n, float f) = 0;

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		virtual void RecalculateViewMatrix() = 0;

	protected:
		glm::mat4 m_ProjectionMatrix{1.0};
		glm::mat4 m_ViewMatrix{1.0};
		glm::mat4 m_ViewProjectionMatrix{1.0};

		glm::vec3 m_Position{0.0f, 0.0f, 0.0f};
	};
}
