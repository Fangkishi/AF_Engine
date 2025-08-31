#include "afpch.h"
#include "GameCameraController.h"

#include "AF/Core/Input.h"
#include "AF/Core/KeyCodes.h"

namespace AF {
	GameCameraController::GameCameraController(float fovy, float aspectRatio)
		: CameraController(), m_Camera(fovy, aspectRatio)
	{

	}

	void GameCameraController::OnUpdate(Timestep ts)
	{
		AF_PROFILE_FUNCTION();

		ts = ts > 1 / 200.0f ? 1 / 200.0f : ts; //根据每帧耗时决定，过滤突变帧
		auto mousePosition = AF::Input::GetMousePosition();
		bool mouseMoved = false;
		if (AF::Input::IsMouseButtonPressed(Mouse::ButtonRight))
		{
			float deltaX = (mousePosition.x - m_CurrentX) * m_Sensitivity * 1.0f / 200.0f;
			float deltaY = (mousePosition.y - m_CurrentY) * m_Sensitivity * 1.0f / 200.0f;

			Pitch(-deltaY);
			Yaw(-deltaX);

			m_CurrentX = mousePosition.x;
			m_CurrentY = mousePosition.y;
			mouseMoved = true;
		}
		else
		{
			// 如果没有按下右键，更新鼠标位置但不旋转相机
			m_CurrentX = mousePosition.x;
			m_CurrentY = mousePosition.y;
		}

		//最终移动方向
		glm::vec3 direction(0.0f);

		auto front = glm::cross(m_Camera.GetUp(), m_Camera.GetRight());

		auto up = m_Camera.GetUp();

		auto right = m_Camera.GetRight();

		if (Input::IsKeyPressed(Key::W))
		{
			direction += front;
		}

		if (Input::IsKeyPressed(Key::S))
		{
			direction -= front;
		}

		if (Input::IsKeyPressed(Key::A))
		{
			direction -= right;
		}

		if (Input::IsKeyPressed(Key::D))
		{
			direction += right;
		}

		if (Input::IsKeyPressed(Key::Space))
		{
			direction += up;
		}

		if (Input::IsKeyPressed(Key::LeftControl))
		{
			direction -= up;
		}

		//归一化
		if (glm::length(direction) != 0)
		{
			direction = glm::normalize(direction);
			glm::vec3 position = m_Camera.GetPosition() + direction * m_CameraTranslationSpeed * (float)ts;
			m_Camera.SetPosition(position);
		}

		if (mouseMoved)
		{
			//仅当鼠标移动但没有位置变化时，手动更新视图矩阵
			m_Camera.RecalculateViewMatrix();
		}

	}

	void GameCameraController::OnEvent(Event& e)
	{
		AF_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(AF_BIND_EVENT_FN(GameCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(AF_BIND_EVENT_FN(GameCameraController::OnWindowResized));
	}

	void GameCameraController::OnResize(float width, float height)
	{
		float aspect = width / height;
		m_Camera.SetAspect(aspect);
		m_Camera.SetProjection(m_Camera.GetFovy(), aspect, m_Camera.GetNear(), m_Camera.GetFar());
	}

	bool GameCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		float deltaZoom = e.GetYOffset() * 0.25f;
		m_Camera.Scale(deltaZoom * m_ScaleSpeed);

		return false;
	}

	bool GameCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		AF_PROFILE_FUNCTION();

		float width = (float)e.GetWidth(), height = (float)e.GetHeight();
		OnResize(width, height);
		return false;
	}

	void GameCameraController::Pitch(float angle)
	{
		m_Pitch += angle;
		if (m_Pitch > 89.0f || m_Pitch < -89.0f)
		{
			m_Pitch -= angle;
			return;
		}

		auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), m_Camera.GetRight());

		m_Camera.SetUp(mat * glm::vec4(m_Camera.GetUp(), 0.0f));
	}

	void GameCameraController::Yaw(float angle)
	{
		auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

		m_Camera.SetUp(mat * glm::vec4(m_Camera.GetUp(), 0.0f));
		m_Camera.SetRight(mat * glm::vec4(m_Camera.GetRight(), 0.0f));
	}
}
