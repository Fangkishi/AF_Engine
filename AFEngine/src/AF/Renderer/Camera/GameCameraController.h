#pragma once
#include "CameraController.h"

namespace AF {
	class GameCameraController : public CameraController
	{
	public:
		GameCameraController(float fovy, float aspectRatio);

		void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;

		virtual void OnResize(float width, float height) override;

		PerspectiveCamera& GetCamera() { return m_Camera; }
		const PerspectiveCamera& GetCamera() const { return m_Camera; }

	protected:
		virtual bool OnMouseScrolled(MouseScrolledEvent& e) override;
		virtual bool OnWindowResized(WindowResizeEvent& e) override;

	private:
		void Pitch(float angle);
		void Yaw(float angle);

	private:
		PerspectiveCamera m_Camera;

		float m_CurrentX = 0.0f, m_CurrentY = 0.0f;

		float m_Pitch = 0.0f;
	};
}
