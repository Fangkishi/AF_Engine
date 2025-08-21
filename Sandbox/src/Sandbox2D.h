#pragma once

#include "AF.h"

class Sandbox2D : public AF::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(AF::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(AF::Event& e) override;
private:
	AF::OrthographicCamera2DController m_CameraController;

	AF::Ref<AF::Shader> m_FlatColorShader;
	AF::Ref<AF::VertexArray> m_SquareVA;

	AF::Ref<AF::Texture2D> m_CheckerboardTexture;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};