#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace AF {

	class Camera {
	public:
		Camera();
		~Camera();

		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		const glm::vec3& GetUp() const { return m_Up; }
		void SetUp(const glm::vec3& up) { m_Up = up; }
		const glm::vec3& GetRight() const { return m_Right; }
		void SetRight(const glm::vec3& right) { m_Right = right; }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		virtual void scale(float deltaScale) = 0;

		void RecalculateViewMatrix();
	public:
		glm::mat4 m_ProjectionMatrix{ 1.0 };
		glm::mat4 m_ViewMatrix{ 1.0 };
		glm::mat4 m_ViewProjectionMatrix{ 1.0 };

		glm::vec3 m_Position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Up{ 0.0f, 1.0f, 0.0f };
		glm::vec3 m_Right{ 1.0f, 0.0f, 0.0f };

		float m_Near = 0.1f;
		float m_Far = 100.0f;
	};

}