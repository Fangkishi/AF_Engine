#include "afpch.h"
#include "CameraController.h"

namespace AF {

	CameraController::CameraController(Ref<Camera> camera)
	{
		m_Camera = camera;
	}

	void CameraController::OnEvent(Event& e)
	{
		AF_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(AF_BIND_EVENT_FN(CameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(AF_BIND_EVENT_FN(CameraController::OnWindowResized));
	}

	void CameraController::OnResize(float width, float height)
	{
		if (Ref<PerspectiveCamera> perspectiveCamera = std::dynamic_pointer_cast<PerspectiveCamera>(m_Camera))
		{
			perspectiveCamera->SetProjection(perspectiveCamera->m_Fovy, width / height, perspectiveCamera->m_Near, perspectiveCamera->m_Far);
		}
		else if (Ref<OrthographicCamera> orthographicCamera = std::dynamic_pointer_cast<OrthographicCamera>(m_Camera))
		{
			// 只更新投影矩阵而不修改边界值
			float aspect = width / height;

			// 基于原始边界值和新宽高比计算投影矩阵
			float originalAspect = (orthographicCamera->m_R - orthographicCamera->m_L) / (orthographicCamera->m_T - orthographicCamera->m_B);

			float left, right, bottom, top;
			if (aspect > originalAspect)
			{
				// 新窗口更宽，上下留黑边
				float newHeight = (orthographicCamera->m_R - orthographicCamera->m_L) / aspect;
				float center = (orthographicCamera->m_T + orthographicCamera->m_B) * 0.5f;
				bottom = center - newHeight * 0.5f;
				top = center + newHeight * 0.5f;
				left = orthographicCamera->m_L;
				right = orthographicCamera->m_R;
			}
			else
			{
				// 新窗口更高，左右留黑边
				float newWidth = (orthographicCamera->m_T - orthographicCamera->m_B) * aspect;
				float center = (orthographicCamera->m_R + orthographicCamera->m_L) * 0.5f;
				left = center - newWidth * 0.5f;
				right = center + newWidth * 0.5f;
				bottom = orthographicCamera->m_B;
				top = orthographicCamera->m_T;
			}

			// 应用当前缩放级别
			float scale = std::pow(2.0f, m_ZoomLevel);
			orthographicCamera->m_ProjectionMatrix = glm::ortho(left * scale, right * scale, bottom * scale, top * scale, orthographicCamera->m_Near, orthographicCamera->m_Far);
			orthographicCamera->m_ViewProjectionMatrix = orthographicCamera->m_ProjectionMatrix * orthographicCamera->m_ViewMatrix;
		}
		else if (Ref<OrthographicCamera2D> orthographicCamera2D = std::dynamic_pointer_cast<OrthographicCamera2D>(m_Camera))
		{
			// 只更新投影矩阵而不修改边界值
			float aspect = width / height;

			// 基于原始边界值和新宽高比计算投影矩阵
			float originalAspect = (orthographicCamera2D->m_R - orthographicCamera2D->m_L) / (orthographicCamera2D->m_T - orthographicCamera2D->m_B);

			float left, right, bottom, top;
			if (aspect > originalAspect)
			{
				// 新窗口更宽，上下留黑边
				float newHeight = (orthographicCamera2D->m_R - orthographicCamera2D->m_L) / aspect;
				float center = (orthographicCamera2D->m_T + orthographicCamera2D->m_B) * 0.5f;
				bottom = center - newHeight * 0.5f;
				top = center + newHeight * 0.5f;
				left = orthographicCamera2D->m_L;
				right = orthographicCamera2D->m_R;
			}
			else
			{
				// 新窗口更高，左右留黑边
				float newWidth = (orthographicCamera2D->m_T - orthographicCamera2D->m_B) * aspect;
				float center = (orthographicCamera2D->m_R + orthographicCamera2D->m_L) * 0.5f;
				left = center - newWidth * 0.5f;
				right = center + newWidth * 0.5f;
				bottom = orthographicCamera2D->m_B;
				top = orthographicCamera2D->m_T;
			}

			 //应用当前缩放级别
			float scale = std::pow(2.0f, m_ZoomLevel);
			orthographicCamera2D->m_ProjectionMatrix = glm::ortho(left * scale, right * scale, bottom * scale, top * scale, -1.0f, 1.0f);
			orthographicCamera2D->m_ViewProjectionMatrix = orthographicCamera2D->m_ProjectionMatrix * orthographicCamera2D->m_ViewMatrix;
		}
	}

	bool CameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		AF_PROFILE_FUNCTION();

		float deltaZoom = e.GetYOffset() * 0.25f;
		m_ZoomLevel -= m_ScaleSpeed * deltaZoom;
		if (std::dynamic_pointer_cast<PerspectiveCamera>(m_Camera))
		{
			m_Camera->scale(deltaZoom);
		}
		else
		{
			m_Camera->scale(m_ZoomLevel);
		}

		return false;
	}

	bool CameraController::OnWindowResized(WindowResizeEvent& e)
	{
		AF_PROFILE_FUNCTION();

		float width = (float)e.GetWidth(), height = (float)e.GetHeight();
		OnResize(width, height);
		return false;
	}

}
