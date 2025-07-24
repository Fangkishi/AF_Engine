#pragma once

#ifdef AF_PLATFORM_WINDOWS
	#ifdef AF_BUILD_DLL
		#define AF_API __declspec(dllexport)
	#else
		#define AF_API __declspec(dllimport)
	#endif
#else
	#error AFEngine only support Windows !
#endif
