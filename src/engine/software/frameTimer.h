#pragma once

#include <chrono>

namespace minity
{
    using Clock = std::chrono::steady_clock;

    class frameTimer
    {
        typename Clock::time_point frameStart;

    private:
        float m_deltaTime = 0.0f;

    public:
        frameTimer()
            : frameStart(Clock::now())
        {
        }

        // returns the latest fps number and stores timeDelta [s]
        uint delta()
        {
            auto now = Clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - frameStart);
            frameStart = now;
            m_deltaTime = elapsed_ms.count() / 1000.0f;
            return elapsed_ms.count() != 0 ? 1000 / (uint)elapsed_ms.count() : 0;
        }

        float deltaTime()
        {
            return m_deltaTime;
        }
    };
} // minity
