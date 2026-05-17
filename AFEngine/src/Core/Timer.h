#pragma once

// Timer —— 高精度计时器，基于 std::chrono::high_resolution_clock

#include <chrono>

namespace AF {

class Timer
{
public:
    Timer() { Reset(); }

    void Reset()
    {
        m_Start = std::chrono::high_resolution_clock::now();
    }

    /// 自上次 Reset 以来的秒数
    float ElapsedSeconds() const
    {
        return std::chrono::duration<float>(
            std::chrono::high_resolution_clock::now() - m_Start).count();
    }

    /// 自上次 Reset 以来的毫秒数
    float ElapsedMillis() const
    {
        return ElapsedSeconds() * 1000.0f;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

} // namespace AF
