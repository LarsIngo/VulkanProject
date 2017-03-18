#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>

#include "VkRenderer.hpp"
#include "CPUTimer.hpp"
#include "InputManager.hpp"
#include "Camera.hpp"
#include "vkTools.hpp"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // +++ INIT +++ //
    unsigned int width = 1920 / 2;
    unsigned int height = 1080 / 2;
    VkRenderer renderer(width, height);
    VkDevice device = renderer.mDevice;
    VkPhysicalDevice physicalDevice = renderer.mPhysicalDevice;
    VkCommandPool graphicsCommandPool = renderer.mGraphicsCommandPool;
    VkCommandPool computeCommandPool = renderer.mComputeCommandPool;
    VkQueue graphicsQueue = renderer.mGraphicsQueue;
    VkQueue computeQueue = renderer.mComputeQueue;

    InputManager inputManager(renderer.mGLFWwindow);

    FrameBuffer frameBuffer(device, physicalDevice, width, height, renderer.mSurfaceFormatKHR.format);
    Camera camera(60.f, &frameBuffer);
    camera.mPosition.z = 0.f;

    VkCommandBuffer graphicsCommandBuffer;
    vkTools::CreateCommandBuffer(device, graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, graphicsCommandBuffer);
    VkCommandBuffer computeCommandBuffer;
    vkTools::CreateCommandBuffer(device, computeCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeCommandBuffer);
    // --- INIT --- //

    // +++ MAIN LOOP +++ //
    float dt = 1.f;
    while (renderer.Running())
    {
        std::cout << "CPU TIMER: " << 1000.f * dt << " ms | FPS: " << 1.f / dt << std::endl;
        glm::clamp(dt, 1.f / 6000.f, 1.f / 60.f);
        CPUTIMER(dt);

        // +++ UPDATE +++ //
        vkTools::BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, computeCommandBuffer);

        vkTools::EndCommandBuffer(computeCommandBuffer);
        vkTools::SubmitCommandBuffer(computeQueue, computeCommandBuffer);
        vkTools::WaitQueue(computeQueue);
        vkTools::ResetCommandBuffer(computeCommandBuffer);
        // --- UPDATE --- //

        // +++ RENDER +++ //
        vkTools::BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, graphicsCommandBuffer);

        FrameBuffer* backBuffer = renderer.SwapBackBuffer();
        backBuffer->TransitionImageLayout(graphicsCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        camera.mpFrameBuffer->Clear(graphicsCommandBuffer, 1.f, 0.f, 0.f, 1.f);
        backBuffer->Copy(graphicsCommandBuffer, camera.mpFrameBuffer);

        backBuffer->TransitionImageLayout(graphicsCommandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        vkTools::EndCommandBuffer(graphicsCommandBuffer);
        vkTools::SubmitCommandBuffer(graphicsQueue, graphicsCommandBuffer);
        vkTools::WaitQueue(graphicsQueue);
        vkTools::ResetCommandBuffer(graphicsCommandBuffer);
        // --- RENDER --- //

        // +++ PRESENET +++ //
        renderer.PresentBackBuffer();
        // --- PRESENET --- //
    }
    // --- MAIN LOOP --- //

    // +++ SHUTDOWN +++ //
    vkTools::FreeCommandBuffer(device, graphicsCommandPool, graphicsCommandBuffer);
    // --- SHUTDOWN --- //

    return 0;
}
