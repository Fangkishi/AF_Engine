#pragma once
#include "CameraController.h"

namespace AF {

	class GameCameraController : public CameraController {
	public:
		GameCameraController(Ref<Camera> camera);

		void OnUpdate(Timestep ts) override;

	private:
		void Pitch(float angle);
		void Yaw(float angle);

	private:
		float m_Pitch = 0.0f;
	};
}