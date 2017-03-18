#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>

#include "VkRenderer.hpp"
#include "CPUTimer.hpp"
#include "InputManager.hpp"
#include "Camera.hpp"
#include "vkTools.hpp"
#include "ParticleSystem.hpp"
#include "Scene.hpp"

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
    VkCommandBuffer graphicsCommandBuffer;
    vkTools::CreateCommandBuffer(device, graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, graphicsCommandBuffer);
    VkCommandBuffer computeCommandBuffer;
    vkTools::CreateCommandBuffer(device, computeCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeCommandBuffer);

    ParticleSystem particleSystem(device, physicalDevice);

    InputManager inputManager(renderer.mGLFWwindow);

    FrameBuffer frameBuffer(device, physicalDevice, width, height, renderer.mSurfaceFormatKHR.format);
    Camera camera(60.f, &frameBuffer);
    camera.mPosition.z = 0.f;

    Scene scene(device, physicalDevice, 1024 * 1024);
    {
        std::vector<Particle> particleList;
        Particle particle;
        float spaceing = 1.f;
        float speed = 0.1f;
        int lenX = 1024;
        int lenY = 1024;
        particle.scale = glm::vec4(spaceing / 2.f, spaceing / 2.f, 0.f, 0.f);
        for (int y = 0; y < lenY; ++y)
        {
            for (int x = 0; x < lenX; ++x)
            {
                particle.position = glm::vec4(x * spaceing, y * spaceing, 0.f, 0.f);
                particle.velocity = -glm::normalize(particle.position + glm::vec4(speed, speed, 0.f, 0.f));
                particle.color = glm::vec4((float)y / lenY, 0.7f, 1.f - (float)x / lenX, 1.f);
                particleList.push_back(particle);
            }
        }
        scene.AddParticles(particleList);
    }
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
