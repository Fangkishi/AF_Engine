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

		void SetViewMatrix(glm::mat4& viewMatrix) { m_ViewMatrix = viewMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjection() const { return m_Projection; }

		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
	};
}
