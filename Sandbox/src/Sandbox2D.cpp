#include "Sandbox2D.h"
#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(16.0f / 9.0f, true), m_SquareColor({0.2f, 0.3f, 0.8f, 1.0f})
{
}

void Sandbox2D::OnAttach()
{
	AF_PROFILE_FUNCTION();

	m_CheckerboardTexture = AF::Texture2D::Create("assets/textures/defaultTexture.jpg");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(AF::Timestep ts)
{
	AF_PROFILE_FUNCTION();

	m_CameraController.OnUpdate(ts);

	AF::Renderer2D::ResetStats();
	{
		AF_PROFILE_SCOPE("Renderer Prep");
		AF::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
		AF::RenderCommand::Clear();
	}

	{
		AF_PROFILE_SCOPE("Renderer Draw");
		AF::Renderer2D::BeginScene(m_CameraController.GetCamera());

		AF::Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, m_SquareColor);
		AF::Renderer2D::DrawQuad({1.1f, 0.0f}, {1.0f, 1.0f}, {0.5f, 0.3f, 0.2f, 1.0f});
		AF::Renderer2D::DrawQuad({0.0f, 0.0f, -0.1f}, {10.0f, 10.0f}, m_CheckerboardTexture);

		AF::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	AF_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = AF::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

	ImGui::End();
}

void Sandbox2D::OnEvent(AF::Event& e)
{
	AF_PROFILE_FUNCTION();

	m_CameraController.OnEvent(e);
}
