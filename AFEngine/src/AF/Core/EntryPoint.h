#pragma once

#ifdef AF_PLATFORM_WINDOWS

extern AF::Application* AF::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	AF::Log::Init();

	AF_PROFILE_BEGIN_SESSION("Startup", "AF-Profile-Startup.json");
	auto app = AF::CreateApplication({ argc, argv });
	AF_PROFILE_END_SESSION();

	AF_PROFILE_BEGIN_SESSION("Runtime", "AF-Profile-Runtime.json");
	app->Run();
	AF_PROFILE_END_SESSION();

	AF_PROFILE_BEGIN_SESSION("Shutdown", "AF-Profile-Shutdown.json");
	delete app;
	AF_PROFILE_END_SESSION();
}

#endif
