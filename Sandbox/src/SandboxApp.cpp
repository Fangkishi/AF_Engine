#include <AF.h>
#include <AF/Core/EntryPoint.h>

#include "Sandbox2D.h"

class ExampleLayer : public AF::Layer
{
public:
	ExampleLayer()
		:Layer("Example")
	{
		m_Camera = AF::CreateRef<AF::PerspectiveCamera>(45.0f, 1.6f / 0.9f, 0.1f, 100.0f);
		m_CameraController = AF::CreateRef<AF::GameCameraController>(m_Camera);

		//m_Camera = new AF::OrthographicCamera2D(-1.6f, 1.6f, -0.9f, 0.9f);
		//m_CameraController = new AF::OrthographicCamera2DController(static_cast<AF::OrthographicCamera2D*>(m_Camera), true);

		//Vertex Array
		m_VertexArray = AF::VertexArray::Create();

		//Vertex Buffer
		float vertices[] = {
			-0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, -1.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, -1.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, -1.0f, 1.0f, 1.0f,
		};
		AF::Ref<AF::VertexBuffer> VertexBuffer;
		VertexBuffer.reset(AF::VertexBuffer::Create(vertices, sizeof(vertices)));

		AF::BufferLayout layout = {
			{ AF::ShaderDataType::Float3, "a_Position" },
			{ AF::ShaderDataType::Float2, "a_Uv" },
		};

		VertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(VertexBuffer);

		//Index Buffer
		//unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
		unsigned int indices[] = {
			// 前面（第一个面）
			0, 1, 2,   // 第一个三角形
			0, 2, 3,   // 第二个三角形

			// 后面（第二个面）  
			4, 5, 6,   // 第一个三角形
			4, 6, 7,   // 第二个三角形
		};
		AF::Ref<AF::IndexBuffer> IndexBuffer;
		IndexBuffer.reset(AF::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		m_VertexArray->SetIndexBuffer(IndexBuffer);

		auto shader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		//m_Shader = AF::Shader::Create("assets/shaders/Test.glsl");

		m_Texture = AF::Texture2D::Create("assets/textures/defaultTexture.jpg");
	}
	
	void OnUpdate(AF::Timestep ts) override
	{
		AF::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		AF::RenderCommand::Clear();

		m_CameraController->OnUpdate(ts);

		AF::Renderer::BeginScene(*m_Camera);

		auto shader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();

		AF::Renderer::Submit(shader, m_VertexArray);

		AF::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{

	}

	void OnEvent(AF::Event& event) override
	{
		m_CameraController->OnEvent(event);
	}
private:
	AF::ShaderLibrary m_ShaderLibrary;
	//AF::Ref<AF::Shader> m_Shader;
	AF::Ref<AF::VertexArray> m_VertexArray;

	AF::Ref<AF::Texture2D> m_Texture;

	AF::Ref<AF::Camera> m_Camera = nullptr;
	AF::Ref<AF::CameraController> m_CameraController = nullptr;
};

class Sandbox : public AF::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{

	}

};

AF::Application* AF::CreateApplication()
{
	return new Sandbox;
}