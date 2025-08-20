#pragma once

#include "AF/Renderer/Camera/Camera.h"
#include "AF/Renderer/Camera/OrthographicCamera.h"
#include "AF/Renderer/Camera/OrthographicCamera2D.h"
#include "AF/Renderer/Camera/PerspectiveCamera.h"
//#include "AF/Core/Timestep.h"

#include "AF/Events/ApplicationEvent.h"
#include "AF/Events/MouseEvent.h"
#include "AF/Events/KeyEvent.h"

#include "AF/Core/Timestep.h"

#include <glm/gtc/matrix_transform.hpp>

namespace AF {

	class CameraController
	{
	public:
		CameraController(Camera* camera);

		virtual void OnUpdate(Timestep ts) = 0;
		virtual void OnEvent(Event& e);

		virtual void OnResize(float width, float height);

		Camera& GetCamera() { return *m_Camera; }
		const Camera& GetCamera() const { return *m_Camera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }

		void setSensitivity(float s) { m_Sensitivity = s; }
		void setScaleSpeed(float s) { m_ScaleSpeed = s; }
		void SetSpeed(float s) { m_CameraTranslationSpeed = s; }
	protected:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	protected:
		float m_ZoomLevel = 2.236f; //CameraTranslationSpeed开根号

		Camera* m_Camera = nullptr;

		float m_CurrentX = 0.0f, m_CurrentY = 0.0f;

		//相机：鼠标灵敏度 缩放速度 移动速度
		float m_Sensitivity = 40.0f;
		float m_ScaleSpeed = 0.2f;
		float m_CameraTranslationSpeed = 5.0f;
	};

}
