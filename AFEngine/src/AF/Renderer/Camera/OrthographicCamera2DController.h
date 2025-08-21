#pragma once

#include "CameraController.h"

namespace AF {

	class OrthographicCamera2DController : public CameraController
	{
	public:
		OrthographicCamera2DController(Ref<OrthographicCamera2D> camera = CreateRef<OrthographicCamera2D>(-1.6f, 1.6f, -0.9f, 0.9f), bool rotation = true);

		void OnUpdate(Timestep ts);

	private:
		bool m_Rotation;

		glm::vec3 m_CameraPosition{ 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_CameraRotationSpeed = 180.0f;
	};

}