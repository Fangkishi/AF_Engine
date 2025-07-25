#pragma once

#ifdef AF_PLATFORM_WINDOWS

extern AF::Application* AF::CreateApplication();

int main(int argc, char** argv)
{
	AF::Log::Init();
	AF_CORE_TRACE("123");
	AF_CRITICAL("123");

	auto app = AF::CreateApplication();
	app->Run();
	delete app;
}

#endif
