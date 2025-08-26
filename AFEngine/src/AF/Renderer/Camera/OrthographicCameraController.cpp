#include "afpch.h"
#include "AF/Renderer/Camera/OrthographicCameraController.h"

#include "AF/Core/Input.h"
#include "AF/Core/KeyCodes.h"

namespace AF {

	OrthographicCameraController::OrthographicCameraController(Ref<OrthographicCamera> camera, bool rotation)
		: CameraController(),
		m_Camera(camera),
		m_Rotation(rotation)
	{
		m_AspectRatio = (m_Camera->GetR() - m_Camera->GetL()) / (m_Camera->GetT() - m_Camera->GetB());
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		AF_PROFILE_FUNCTION();

		glm::vec3 cameraPosition = m_Camera->GetPosition();

		float speed = m_Camera->GetZoomLevel() * m_CameraTranslationSpeed;

		if (Input::IsKeyPressed(Key::A))
		{
			cameraPosition.x -= cos(glm::radians(m_CameraRotation)) * speed * ts;
			cameraPosition.y -= sin(glm::radians(m_CameraRotation)) * speed * ts;
		}
		else if (Input::IsKeyPressed(Key::D))
		{
			cameraPosition.x += cos(glm::radians(m_CameraRotation)) * speed * ts;
			cameraPosition.y += sin(glm::radians(m_CameraRotation)) * speed * ts;
		}

		if (Input::IsKeyPressed(Key::W))
		{
			cameraPosition.x += -sin(glm::radians(m_CameraRotation)) * speed * ts;
			cameraPosition.y += cos(glm::radians(m_CameraRotation)) * speed * ts;
		}
		else if (Input::IsKeyPressed(Key::S))
		{
			cameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * speed * ts;
			cameraPosition.y -= cos(glm::radians(m_CameraRotation)) * speed * ts;
		}

		Ref<OrthographicCamera> camera = std::static_pointer_cast<OrthographicCamera>(m_Camera);

		camera->SetPosition(cameraPosition);

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(Key::Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;
			if (Input::IsKeyPressed(Key::E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			camera->SetRotation(m_CameraRotation);
		}
	}


	void OrthographicCameraController::OnEvent(Event& e)
	{
		AF_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(AF_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(AF_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}


	void OrthographicCameraController::OnResize(float width, float height)
	{
		m_AspectRatio = width / height;
		m_Camera->SetProjection(-m_AspectRatio * m_Camera->GetZoomLevel(), m_AspectRatio * m_Camera->GetZoomLevel(), -m_Camera->GetZoomLevel(), m_Camera->GetZoomLevel());
	}


	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		AF_PROFILE_FUNCTION();

		float deltaZoom = e.GetYOffset() * 0.25f;
		float zoomLevel = m_Camera->GetZoomLevel();
		zoomLevel *= (1.0f - deltaZoom * m_ScaleSpeed);
		zoomLevel = std::clamp(zoomLevel, 0.25f, 4.0f);
		m_Camera->SetZoomLevel(zoomLevel);
		m_Camera->SetProjection(-m_AspectRatio * m_Camera->GetZoomLevel(), m_AspectRatio * m_Camera->GetZoomLevel(), -m_Camera->GetZoomLevel(), m_Camera->GetZoomLevel());
		return false;
	}


	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		AF_PROFILE_FUNCTION();

		OnResize((float)e.GetWidth(), (float)e.GetHeight());
		return false;
	}

}