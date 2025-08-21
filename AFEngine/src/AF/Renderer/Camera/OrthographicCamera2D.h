#pragma once

#include "Camera.h"

namespace AF {

	class OrthographicCamera2D : public Camera
	{
	public:
		OrthographicCamera2D(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		virtual void scale(float deltaScale)override;
	private:
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
