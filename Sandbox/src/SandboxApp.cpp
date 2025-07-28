#include <AF.h>

class ExampleLayer : public AF::Layer
{
public:
	ExampleLayer()
		:Layer("Example")
	{

	}
	
	void OnUpdate() override
	{
		AF_INFO("ExampleLayer::Update");
	}

	void OnEvent(AF::Event& event) override
	{
		AF_TRACE("{0}", event.ToString());
	}
};

class Sandbox : public AF::Application
{
public:
	Sandbox()
	{
		PushLayer(new AF::ImGuiLayer());
	}

	~Sandbox()
	{

	}

};

AF::Application* AF::CreateApplication()
{
	return new Sandbox;
}