#pragma once

#include "Core.h"

namespace AF {

	class AF_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	Application* CreateApplication();
}


