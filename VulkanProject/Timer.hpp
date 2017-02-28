#pragma once

#include <chrono>

// Profiler.
class Timer {
    public:
        // Constructor.
        Timer(float& dt)
        {
            mStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            mDt = &dt;
        }

        // Destructor.
        ~Timer()
        {
            long long deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - mStart;
            *mDt = static_cast<float>(deltaTime) / 1000.f;
        }

    private:
        long long mStart;
        float* mDt;

};

#define TIMER(dt) Timer instance(dt)
