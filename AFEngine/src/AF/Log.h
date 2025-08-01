#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace AF {

	class AF_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define AF_CORE_TRACE(...)    ::AF::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define AF_CORE_INFO(...)     ::AF::Log::GetCoreLogger()->info(__VA_ARGS__)
#define AF_CORE_WARN(...)     ::AF::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define AF_CORE_ERROR(...)    ::AF::Log::GetCoreLogger()->error(__VA_ARGS__)
#define AF_CORE_CRITICAL(...) ::AF::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define AF_TRACE(...)         ::AF::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AF_INFO(...)          ::AF::Log::GetClientLogger()->info(__VA_ARGS__)
#define AF_WARN(...)          ::AF::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AF_ERROR(...)         ::AF::Log::GetClientLogger()->error(__VA_ARGS__)
#define AF_CRITICAL(...)      ::AF::Log::GetClientLogger()->critical(__VA_ARGS__)