#pragma once

#include "AF/Renderer/Camera/CameraBase.h"
#include "AF/Renderer/Camera/OrthographicCamera.h"
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
		CameraController() {};
		virtual ~CameraController() = default;

		virtual void OnUpdate(Timestep ts) = 0;
		virtual void OnEvent(Event& e) = 0;

		virtual void OnResize(float width, float height) = 0;

		void setSensitivity(float s) { m_Sensitivity = s; }
		void setScaleSpeed(float s) { m_ScaleSpeed = s; }
		void SetSpeed(float s) { m_CameraTranslationSpeed = s; }
	protected:
		virtual bool OnMouseScrolled(MouseScrolledEvent& e) = 0;
		virtual bool OnWindowResized(WindowResizeEvent& e) = 0;
	protected:
		//相机：鼠标灵敏度 缩放速度 移动速度
		float m_Sensitivity = 40.0f;
		float m_ScaleSpeed = 0.2f;
		float m_CameraTranslationSpeed = 5.0f;
	};

}
