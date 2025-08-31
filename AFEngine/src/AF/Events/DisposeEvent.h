#pragma once

#include "Event.h"

namespace AF {
	class DisposeEvent : public Event
	{
	public:
		DisposeEvent(void* Target)
			: mTarget(Target)
		{
		}

		void* GetTarget() const { return mTarget; }

		EVENT_CLASS_TYPE(Dispose)
		EVENT_CLASS_CATEGORY(EventCategoryDispose)

	private:
		void* mTarget = nullptr;;
	};
}
