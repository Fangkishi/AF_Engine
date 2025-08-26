#pragma once

#include "Camera.h"

namespace AF {

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		virtual void SetProjection(float left, float right, float bottom, float top) override;

		float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const float& GetL() const { return m_L; }
		void SetL(const float& l) { m_L = l; }
		const float& GetR() const { return m_R; }
		void SetR(const float& r) { m_R = r; }
		const float& GetB() const { return m_B; }
		void SetB(const float& b) { m_B = b; }
		const float& GetT() const { return m_T; }
		void SetT(const float& t) { m_T = t; }

		void RecalculateViewMatrix();
	public:
		float m_L = 0.0f;
		float m_R = 0.0f;
		float m_B = 0.0f;
		float m_T = 0.0f;

		float m_Rotation = 0.0f;
		float m_Scale{ 0.0f };
	};

}
