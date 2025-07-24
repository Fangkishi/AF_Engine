#include <AF.h>

class Sandbox : public AF::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}

};

AF::Application* AF::CreateApplication()
{
	return new Sandbox;
}