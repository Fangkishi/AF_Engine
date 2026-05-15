#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/std.h>

#include <memory>

namespace AF {

class Log
{
public:
    static void Init();
    static void Shutdown();

    static std::shared_ptr<spdlog::logger>& GetLogger();

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

} // namespace AF

#define AF_LOG_TRACE(...)    ::AF::Log::GetLogger()->trace(__VA_ARGS__)
#define AF_LOG_INFO(...)     ::AF::Log::GetLogger()->info(__VA_ARGS__)
#define AF_LOG_WARN(...)     ::AF::Log::GetLogger()->warn(__VA_ARGS__)
#define AF_LOG_ERROR(...)    ::AF::Log::GetLogger()->error(__VA_ARGS__)
#define AF_LOG_CRITICAL(...) ::AF::Log::GetLogger()->critical(__VA_ARGS__)
