#pragma once

#include "CameraController.h"

namespace AF {

	class OrthographicCamera2DController : public CameraController
	{
	public:
		OrthographicCamera2DController(OrthographicCamera2D* camera, bool rotation = false);

		void OnUpdate(Timestep ts);

	private:
		bool m_Rotation;

		float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_CameraRotationSpeed = 180.0f;
	};

}