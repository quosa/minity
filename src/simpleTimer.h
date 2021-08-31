#include <chrono>

namespace jku
{
    using Clock = std::chrono::steady_clock;

    class frametimer
    {
        typename Clock::time_point frameStart;

    private:
        float delta_time = 0.0f;

    public:
        frametimer()
            : frameStart(Clock::now())
        {
        }
        // returns the latest fps number and stores timeDelta [s]
        uint delta()
        {
            auto now = Clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - frameStart);
            frameStart = now;
            delta_time = elapsed_ms.count() / 1000.0f;
            return elapsed_ms.count() != 0 ? 1000 / (uint)elapsed_ms.count() : 0;
        }
        float deltaTime()
        {
            return delta_time;
        }
    };
}
