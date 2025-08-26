#pragma once

#include "CameraController.h"

namespace AF {

	class OrthographicCameraController : public CameraController
	{
	public:
		OrthographicCameraController(Ref<OrthographicCamera> camera = CreateRef<OrthographicCamera>(-1.6f, 1.6f, -0.9f, 0.9f), bool rotation = true);

		void OnUpdate(Timestep ts);
		virtual void OnEvent(Event& e) override;

		virtual void OnResize(float width, float height) override;

		Ref<OrthographicCamera> GetCamera() { return m_Camera; }
		const Ref<OrthographicCamera> GetCamera() const { return m_Camera; }
	protected:
		virtual bool OnMouseScrolled(MouseScrolledEvent& e) override;
		virtual bool OnWindowResized(WindowResizeEvent& e) override;
	private:
		float m_AspectRatio;

		Ref<OrthographicCamera> m_Camera;

		bool m_Rotation;

		float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_CameraRotationSpeed = 180.0f;
	};

}