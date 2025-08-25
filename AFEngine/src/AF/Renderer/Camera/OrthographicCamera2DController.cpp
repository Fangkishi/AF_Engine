#include "afpch.h"
#include "AF/Renderer/Camera/OrthographicCamera2DController.h"

#include "AF/Core/Input.h"
#include "AF/Core/KeyCodes.h"

namespace AF {

	OrthographicCamera2DController::OrthographicCamera2DController(Ref<OrthographicCamera2D> camera, bool rotation)
		: CameraController(camera), m_Rotation(rotation)
	{
		camera->SetPosition(m_CameraPosition);
	}

	void OrthographicCamera2DController::OnUpdate(Timestep ts)
	{
		AF_PROFILE_FUNCTION();

		glm::vec3 cameraPosition = m_Camera->GetPosition();

		float speed = std::pow(2.0f, m_ZoomLevel) * m_CameraTranslationSpeed;

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

		Ref<OrthographicCamera2D> camera2D = std::static_pointer_cast<OrthographicCamera2D>(m_Camera);

		camera2D->SetPosition(cameraPosition);

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

			camera2D->SetRotation(m_CameraRotation);
		}


	}

}