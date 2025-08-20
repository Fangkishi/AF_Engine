#include "afpch.h"
#include "AF/Renderer/Camera/OrthographicCamera2DController.h"

#include "AF/Input.h"
#include "AF/Core/KeyCodes.h"

namespace AF {

	OrthographicCamera2DController::OrthographicCamera2DController(OrthographicCamera2D* camera, bool rotation)
		: CameraController(camera), m_Rotation(rotation)
	{

	}

	void OrthographicCamera2DController::OnUpdate(Timestep ts)
	{
		glm::vec3 cameraPosition = m_Camera->GetPosition();

		if (Input::IsKeyPressed(Key::A))
		{
			cameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			cameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(Key::D))
		{
			cameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			cameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		if (Input::IsKeyPressed(Key::W))
		{
			cameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			cameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(Key::S))
		{
			cameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			cameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

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

			if (OrthographicCamera2D* camera2D = dynamic_cast<OrthographicCamera2D*>(m_Camera))
			{
				camera2D->SetRotation(m_CameraRotation);
			}
		}

		if (m_Camera)
		{
			m_Camera->SetPosition(cameraPosition);
		}

		m_CameraTranslationSpeed = std::pow(2.0f, m_ZoomLevel);
	}

}