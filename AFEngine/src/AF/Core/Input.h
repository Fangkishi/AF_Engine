#pragma once

#include "AF/Core/KeyCodes.h"
#include "AF/Core/MouseCodes.h"

#include <glm/glm.hpp>

namespace AF {

	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};

}