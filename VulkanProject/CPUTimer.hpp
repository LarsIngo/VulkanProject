#pragma once

#include <chrono>

// CPU timer.
class CPUTimer {
    public:
        // Constructor.
        CPUTimer(float& dt)
        {
            mStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            mDt = &dt;
        }

        // Destructor.
        ~CPUTimer()
        {
            long long deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - mStart;
            *mDt = static_cast<float>(deltaTime) / 1000.f;
        }

    private:
        long long mStart;
        float* mDt;

};

#define CPUTIMER(dt) CPUTimer instance(dt)
