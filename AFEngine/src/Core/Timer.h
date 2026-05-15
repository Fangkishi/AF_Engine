#pragma once

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

    float ElapsedSeconds() const
    {
        return std::chrono::duration<float>(
            std::chrono::high_resolution_clock::now() - m_Start).count();
    }

    float ElapsedMillis() const
    {
        return ElapsedSeconds() * 1000.0f;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

} // namespace AF
