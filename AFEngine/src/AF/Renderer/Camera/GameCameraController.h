#pragma once
#include "CameraController.h"

namespace AF {

	class GameCameraController : public CameraController {
	public:
		GameCameraController(Ref<PerspectiveCamera> camera);

		void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;

		virtual void OnResize(float width, float height) override;

		Ref<PerspectiveCamera> GetCamera() { return m_Camera; }
		const Ref<PerspectiveCamera> GetCamera() const { return m_Camera; }
	protected:
		virtual bool OnMouseScrolled(MouseScrolledEvent& e) override;
		virtual bool OnWindowResized(WindowResizeEvent& e) override;
	private:
		void Pitch(float angle);
		void Yaw(float angle);
	private:
		Ref<PerspectiveCamera> m_Camera;

		float m_Pitch = 0.0f;
	};
}