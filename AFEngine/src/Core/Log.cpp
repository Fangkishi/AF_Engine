#include "Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace AF {

std::shared_ptr<spdlog::logger> Log::s_Logger;

void Log::Init()
{
#ifdef AF_PLATFORM_WINDOWS
    SetConsoleOutputCP(65001);
#endif

    spdlog::set_pattern("%^[%T] %n: %v%$");
    s_Logger = spdlog::stdout_color_mt("AFEngine");
    s_Logger->set_level(spdlog::level::trace);
}

void Log::Shutdown()
{
    s_Logger.reset();
    spdlog::shutdown();
}

std::shared_ptr<spdlog::logger>& Log::GetLogger()
{
    return s_Logger;
}

} // namespace AF
