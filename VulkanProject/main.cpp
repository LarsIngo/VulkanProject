#include <crtdbg.h>

#include "VkRenderer.hpp"
#include "CPUTimer.hpp"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // +++ INIT +++ //
    VkRenderer renderer(800, 600);
    // --- INIT --- //

    // +++ MAIN LOOP +++ //
    float dt = 0.f;
    while (renderer.Running())
    {
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
