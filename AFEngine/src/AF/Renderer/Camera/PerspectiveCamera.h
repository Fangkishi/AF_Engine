#pragma once

#include "Camera.h"

namespace AF {
	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(float fovy, float aspect, float n, float f);

		void SetProjection(float fovy, float aspect, float n, float f);

		void scale(float deltaScale)override;
	public:
		float m_Fovy = 0.0f;
		float m_Aspect = 0.0f;
	};

}