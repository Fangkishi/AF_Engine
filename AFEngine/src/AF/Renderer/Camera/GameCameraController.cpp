#include "afpch.h"
#include "GameCameraController.h"

#include "AF/Core/Input.h"
#include "AF/Core/KeyCodes.h"

namespace AF {

	GameCameraController::GameCameraController(Ref<Camera> camera)
		:CameraController(camera)
	{

	}

	void GameCameraController::OnUpdate(Timestep ts)
	{
		AF_PROFILE_FUNCTION();

		ts = ts > 1 / 200.0f ? 1 / 200.0f : ts;//根据每帧耗时决定，过滤突变帧
		auto [xpos, ypos] = AF::Input::GetMousePosition();
		bool mouseMoved = false;
		if (AF::Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
			float deltaX = (xpos - m_CurrentX) * m_Sensitivity * 1.0f /200.0f;
			float deltaY = (ypos - m_CurrentY) * m_Sensitivity * 1.0f / 200.0f;

			Pitch(-deltaY);
			Yaw(-deltaX);

			m_CurrentX = xpos;
			m_CurrentY = ypos;
			mouseMoved = true;
		}
		else {
			// 如果没有按下右键，更新鼠标位置但不旋转相机
			m_CurrentX = xpos;
			m_CurrentY = ypos;
		}

		//最终移动方向
		glm::vec3 direction(0.0f);

		auto front = glm::cross(GetCamera().GetUp(), GetCamera().GetRight());

		auto up = GetCamera().GetUp();

		auto right = GetCamera().GetRight();

		if (Input::IsKeyPressed(Key::W)) {
			direction += front;
		}

		if (Input::IsKeyPressed(Key::S)) {
			direction -= front;
		}

		if (Input::IsKeyPressed(Key::A)) {
			direction -= right;
		}

		if (Input::IsKeyPressed(Key::D)) {
			direction += right;
		}

		if (Input::IsKeyPressed(Key::Space)) {
			direction += up;
		}

		if (Input::IsKeyPressed(Key::LeftControl)) {
			direction -= up;
		}

		//归一化
		if (glm::length(direction) != 0) {
			direction = glm::normalize(direction);
			glm::vec3 position = GetCamera().GetPosition() + direction * m_CameraTranslationSpeed * (float)ts;
			GetCamera().SetPosition(position);
		}
		
		if (mouseMoved) {
			//仅当鼠标移动但没有位置变化时，手动更新视图矩阵
			GetCamera().RecalculateViewMatrix();
		}

	}

	void GameCameraController::Pitch(float angle)
	{
		m_Pitch += angle;
		if (m_Pitch > 89.0f || m_Pitch < -89.0f) {
			m_Pitch -= angle;
			return;
		}

		auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), m_Camera->GetRight());

		m_Camera->SetUp(mat * glm::vec4(m_Camera->GetUp(), 0.0f));
	}

	void GameCameraController::Yaw(float angle)
	{
		auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

		m_Camera->SetUp(mat * glm::vec4(m_Camera->GetUp(), 0.0f));
		m_Camera->SetRight(mat * glm::vec4(m_Camera->GetRight(), 0.0f));
	}

}