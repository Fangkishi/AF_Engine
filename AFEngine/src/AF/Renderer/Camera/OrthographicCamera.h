#pragma once

#include "Camera.h"

namespace AF {

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top, float n, float f);

		void SetProjection(float left, float right, float bottom, float top, float n, float f);

		virtual void scale(float deltaScale) override;
	public:
		float m_L = 0.0f;
		float m_R = 0.0f;
		float m_B = 0.0f;
		float m_T = 0.0f;

		float m_Scale{ 0.0f };
	};

}
