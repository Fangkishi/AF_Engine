#pragma once

#ifdef AF_PLATFORM_WINDOWS

extern AF::Application* AF::CreateApplication();

int main(int argc, char** argv)
{
	auto app = AF::CreateApplication();
	app->Run();
	delete app;
}

#endif
