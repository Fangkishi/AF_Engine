#include "afpch.h"
#include "CameraController.h"

namespace AF {

	CameraController::CameraController(Camera* camera)
	{
		m_Camera = camera;
	}

	void CameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(AF_BIND_EVENT_FN(CameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(AF_BIND_EVENT_FN(CameraController::OnWindowResized));
	}

	void CameraController::OnResize(float width, float height)
	{
		if (PerspectiveCamera* perspectiveCamera = dynamic_cast<PerspectiveCamera*>(m_Camera))
		{
			perspectiveCamera->SetProjection(perspectiveCamera->m_Fovy, width / height, perspectiveCamera->m_Near, perspectiveCamera->m_Far);
		}
		else if (OrthographicCamera* orthographicCamera = dynamic_cast<OrthographicCamera*>(m_Camera))
		{
			float aspect = width / height;
			orthographicCamera->SetProjection(-aspect * m_ZoomLevel, aspect * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, orthographicCamera->m_Near, orthographicCamera->m_Far);
		}
		else if (OrthographicCamera2D* orthographicCamera2D = dynamic_cast<OrthographicCamera2D*>(m_Camera))
		{
			float aspect = width / height;
			orthographicCamera2D->SetProjection(-aspect * m_ZoomLevel, aspect * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		}
	}

	bool CameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		float deltaZoom = -e.GetYOffset() * 0.25f;
		m_ZoomLevel -= m_ScaleSpeed * deltaZoom;

		m_Camera->scale(-m_ScaleSpeed * deltaZoom);

		return false;
	}

	bool CameraController::OnWindowResized(WindowResizeEvent& e)
	{
		float width = (float)e.GetWidth(), height = (float)e.GetHeight();
		OnResize(width, height);
		return false;
	}

}
