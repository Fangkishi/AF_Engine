#pragma once

#include "Camera.h"

namespace AF {
	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(float fovy, float aspect, float n, float f);

		virtual void SetProjection(float fovy, float aspect, float n, float f) override;

		void Scale(float deltaScale);

		const glm::vec3& GetUp() const { return m_Up; }
		void SetUp(const glm::vec3& up) { m_Up = up; }

		const glm::vec3& GetRight() const { return m_Right; }
		void SetRight(const glm::vec3& right) { m_Right = right; }

		const float& GetFovy() const { return m_Fovy; }
		void SetFovy(const float& fovy) { m_Fovy = fovy; }

		const float& GetAspect() const { return m_Aspect; }
		void SetAspect(const float& aspect) { m_Aspect = aspect; }

		const float& GetNear() const { return m_Near; }
		void SetNear(const float& n) { m_Near = n; }

		const float& GetFar() const { return m_Far; }
		void SetFar(const float& f) { m_Far = f; }

		virtual void RecalculateViewMatrix() override;
	private:
		glm::vec3 m_Up{ 0.0f, 1.0f, 0.0f };
		glm::vec3 m_Right{ 1.0f, 0.0f, 0.0f };

		float m_Fovy = 0.0f;
		float m_Aspect = 0.0f;

		float m_Near = 0.1f;
		float m_Far = 100.0f;
	};

}