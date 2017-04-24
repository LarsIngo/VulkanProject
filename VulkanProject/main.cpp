#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>

#include "VkRenderer.hpp"
#include "CPUTimer.hpp"
#include "VkTimer.hpp"
#include "InputManager.hpp"
#include "Camera.hpp"
#include "vkTools.hpp"
#include "ParticleRenderSystem.hpp"
#include "ParticleUpdateSystem.hpp"
#include "Scene.hpp"
#include "Profiler.hpp"

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
    VkRenderPass renderPass = renderer.mRenderPass;
    VkCommandBuffer graphicsCommandBuffer;
    vkTools::CreateCommandBuffer(device, graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, graphicsCommandBuffer);
    VkCommandBuffer computeCommandBuffer;
    vkTools::CreateCommandBuffer(device, computeCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeCommandBuffer);

    VkCommandBuffer transferCommandBuffer = vkTools::BeginSingleTimeCommand(device, renderer.mTransferCommandPool);

    ParticleUpdateSystem particleUpdateSystem(device, physicalDevice);
    ParticleRenderSystem particleRenderSystem(device, physicalDevice, width, height, renderer.mSurfaceFormatKHR.format, renderPass);

    InputManager inputManager(renderer.mGLFWwindow);

    FrameBuffer frameBuffer(device, physicalDevice, width, height, renderer.mSurfaceFormatKHR.format, renderPass);
    Camera camera(60.f, &frameBuffer);
    camera.mPosition.z = -5.f;

    int lenX = 256;
    int lenY = 256;
    Scene scene(device, physicalDevice, lenX * lenY);
    {
        std::vector<Particle> particleList;
        Particle particle;
        float spaceing = 1.f;
        float speed = 0.1f;
        particle.scale = glm::vec4(spaceing / 2.f, spaceing / 2.f, 0.f, 0.f);
        for (int y = 0; y < lenY; ++y)
        {
            for (int x = 0; x < lenX; ++x)
            {
                particle.position = glm::vec4(x * spaceing, y * spaceing, 0.f, 0.f);
                //particle.velocity = -glm::normalize(particle.position + glm::vec4(speed, speed, 0.f, 0.f));
                particle.velocity = glm::vec4(0.f, 0.f, 0.f, 0.f);
                particle.color = glm::vec4((float)y / lenY, 0.7f, 1.f - (float)x / lenX, 1.f);
                particleList.push_back(particle);
            }
        }
        scene.AddParticles(transferCommandBuffer, particleList);
    }
    vkTools::EndSingleTimeCommand(device, renderer.mTransferCommandPool, renderer.mTransferQueue, transferCommandBuffer);
    // --- INIT --- //

    // +++ MAIN LOOP +++ //
    {
        float dt = 0.f;
        float totalTime = 0.f;
        unsigned int frameCount = 0;
        VkTimer gpuComputeTimer(device, physicalDevice);
        VkTimer gpuGraphicsTimer(device, physicalDevice);
        Profiler profiler(1600, 200);
        while (renderer.Running())
        {
            //glm::clamp(dt, 1.f / 6000.f, 1.f / 60.f);
            bool syncComputeGraphics = inputManager.KeyPressed(GLFW_KEY_F1);
            bool gpuProfile = inputManager.KeyPressed(GLFW_KEY_F2);
            {
                CPUTIMER(dt);

                // +++ UPDATE +++ //
                vkTools::WaitQueue(computeQueue);
                vkTools::ResetCommandBuffer(computeCommandBuffer);
                vkTools::BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, computeCommandBuffer);
                if (gpuProfile) gpuComputeTimer.Start(computeCommandBuffer);

                camera.Update(20.f, 2.f, dt, &inputManager);
                particleUpdateSystem.Update(computeCommandBuffer, &scene, dt);

                if (gpuProfile) gpuComputeTimer.Stop(computeCommandBuffer);
                vkTools::EndCommandBuffer(computeCommandBuffer);
                vkTools::SubmitCommandBuffer(computeQueue, { computeCommandBuffer }, { renderer.mComputeCompleteSemaphore });
                // SYNC_COMPUTE_GRAPHICS
                if (syncComputeGraphics) vkTools::WaitQueue(computeQueue);
                // --- UPDATE --- //

                // +++ RENDER +++ //
                FrameBuffer* backBuffer = renderer.SwapBackBuffer();

                vkTools::WaitQueue(graphicsQueue);
                vkTools::ResetCommandBuffer(graphicsCommandBuffer);
                vkTools::BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, graphicsCommandBuffer);
                if (gpuProfile) gpuGraphicsTimer.Start(graphicsCommandBuffer);

                backBuffer->TransitionImageLayout(graphicsCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                camera.mpFrameBuffer->Clear(graphicsCommandBuffer, 0.2f, 0.2f, 0.2f);
                particleRenderSystem.Render(graphicsCommandBuffer, &scene, &camera);
                backBuffer->Copy(graphicsCommandBuffer, camera.mpFrameBuffer);
                backBuffer->TransitionImageLayout(graphicsCommandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

                if (gpuProfile) gpuGraphicsTimer.Stop(graphicsCommandBuffer);
                vkTools::EndCommandBuffer(graphicsCommandBuffer);
                vkTools::SubmitCommandBuffer(graphicsQueue, { graphicsCommandBuffer }, {}, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { renderer.mComputeCompleteSemaphore });
                // --- RENDER --- //

                // +++ PRESENET +++ //
                vkTools::WaitQueue(computeQueue);
                vkTools::WaitQueue(graphicsQueue);
                renderer.PresentBackBuffer();
                // --- PRESENET --- //
            }
            // +++ PROFILING +++ //
            ++frameCount;
            totalTime += dt;
            if (gpuProfile)
            {
                float computeTime = 1.f / 1000000.f * gpuComputeTimer.GetDeltaTime();
                float graphicsTime = 1.f / 1000000.f * gpuGraphicsTimer.GetDeltaTime();
                std::cout << "GPU(Total) : " << computeTime + graphicsTime << " ms | GPU(Compute): " << computeTime << " ms | GPU(Graphics) : " << graphicsTime << " ms" << std::endl;
                profiler.Rectangle(gpuComputeTimer.GetBeginTime(), 1, gpuComputeTimer.GetDeltaTime(), 1, 0.f, 0.f, 1.f);
                profiler.Rectangle(gpuGraphicsTimer.GetBeginTime(), 0, gpuGraphicsTimer.GetDeltaTime(), 1, 0.f, 1.f, 0.f);
                VkCommandBuffer resetTimerCommandBuffer = vkTools::BeginSingleTimeCommand(device, renderer.mTransferCommandPool);
                gpuComputeTimer.Reset(resetTimerCommandBuffer);
                gpuGraphicsTimer.Reset(resetTimerCommandBuffer);
                vkTools::EndSingleTimeCommand(device, renderer.mTransferCommandPool, renderer.mTransferQueue, resetTimerCommandBuffer);
            }
            if (inputManager.KeyPressed(GLFW_KEY_F3))
            {
                std::cout << "CPU(Average delta time) : " << totalTime / frameCount * 1000.f << " ms" << std::endl;
            }
            // --- PROFILING --- //
        }
    }
    // --- MAIN LOOP --- //

    // +++ SHUTDOWN +++ //
    vkTools::WaitQueue(graphicsQueue);
    vkTools::WaitQueue(computeQueue);
    vkTools::FreeCommandBuffer(device, graphicsCommandPool, graphicsCommandBuffer);
    vkTools::FreeCommandBuffer(device, computeCommandPool, computeCommandBuffer);
    // --- SHUTDOWN --- //

    return 0;
}
