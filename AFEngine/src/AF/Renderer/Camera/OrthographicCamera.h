#pragma once

#include "CameraBase.h"

namespace AF {

	class OrthographicCamera : public CameraBase
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		virtual void SetProjection(float left, float right, float bottom, float top) override;

		float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }


		void RecalculateViewMatrix();
	private:
		float m_Rotation = 0.0f;
		float m_Scale{ 0.0f };
	};

}
