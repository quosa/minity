#include <chrono>

namespace jku
{
    using Clock = std::chrono::steady_clock;

    class frametimer
    {
        typename Clock::time_point frameStart;
    public:
        frametimer()
            : frameStart(Clock::now())
        {}
        uint delta()
        {
            auto now = Clock::now();
    		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - frameStart);
		    frameStart = now;
            return elapsed_ms.count() != 0 ? 1000 / (uint)elapsed_ms.count() : 0;
        }
    };
}