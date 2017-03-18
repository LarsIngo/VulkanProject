#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>

#include "VkRenderer.hpp"
#include "CPUTimer.hpp"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // +++ INIT +++ //
    unsigned int width = 1920 / 2;
    unsigned int height = 1080 / 2;
    VkRenderer renderer(width, height);
    // --- INIT --- //

    // +++ MAIN LOOP +++ //
    float dt = 1.f;
    while (renderer.Running())
    {
        std::cout << "CPU TIMER: " << 1000.f * dt << " ms | FPS: " << 1.f / dt << std::endl;
        glm::clamp(dt, 1.f / 6000.f, 1.f / 60.f);
        CPUTIMER(dt);
        // +++ UPDATE +++ //
        // --- UPDATE --- //

        // +++ RENDER +++ //
        // --- RENDER --- //

        // +++ PRESENET +++ //
        // --- PRESENET --- //
    }
    // --- MAIN LOOP --- //

    // +++ SHUTDOWN +++ //
    // --- SHUTDOWN --- //

    return 0;
}
