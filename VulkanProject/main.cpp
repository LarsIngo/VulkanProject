#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>

#include "VkRenderer.hpp"
#include "CPUTimer.hpp"
#include "VkTimer.hpp"
#include "InputManager.hpp"
#include "Camera.hpp"
#include "FrameBuffer.hpp"
#include "vkTools.hpp"
#include "ParticleRenderSystem.hpp"
#include "ParticleUpdateSystem.hpp"
#include "Scene.hpp"
#include "Profiler.hpp"

#define SKIP_TIME_NANO 5000000000

#define PROFILE_FRAME_COUNT 1000

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
    VkSemaphore graphicsCompleteSemaphore, computeCompleteSemaphore;
    vkTools::CreateVkSemaphore(device, graphicsCompleteSemaphore);
    vkTools::CreateVkSemaphore(device, computeCompleteSemaphore);

    VkCommandBuffer graphicsCommandBuffer;
    vkTools::CreateCommandBuffer(device, graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, graphicsCommandBuffer);
    VkCommandBuffer computeCommandBuffer;
    vkTools::CreateCommandBuffer(device, computeCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeCommandBuffer);

    VkRenderPass renderPass = renderer.mRenderPass;

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
        double startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        double currentTime = 0.0;
        double totalTime = 0.0;
        double totalMeasureTime = 0.0;
        float mt = 0.f;
        float dt = 0.f;
        double profileFrames[PROFILE_FRAME_COUNT];
        for (int i = 0; i < PROFILE_FRAME_COUNT; ++i)
            profileFrames[i] = 0.0;
        double averageTime = 0.0;

        std::cout << "+++ Skip time: " << SKIP_TIME_NANO << " nanoseconds. (Wait for program to stabilize) +++" << std::endl;
        std::cout << "Hold F1 to sync compute/graphics. " << std::endl;
        std::cout << "Hold F2 to profile. " << std::endl;
        std::cout << "Hold F3 to show average frame time. " << std::endl;
        unsigned int frameCount = 0;
        VkTimer gpuComputeTimer(device, physicalDevice);
        VkTimer gpuGraphicsTimer(device, physicalDevice);
        Profiler profiler(1600, 200);
        while (renderer.Running())
        {
            //glm::clamp(dt, 1.f / 6000.f, 1.f / 60.f);
            bool syncComputeGraphics = inputManager.KeyPressed(GLFW_KEY_F1);
            //bool gpuProfile = inputManager.KeyPressed(GLFW_KEY_F2);
            {
                double lastTime = currentTime;
                currentTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                dt = currentTime - lastTime;
                totalTime = currentTime - startTime;

                CPUTIMER(mt);

                // +++ UPDATE +++ //
                //vkTools::WaitQueue(computeQueue);
                vkTools::ResetCommandBuffer(computeCommandBuffer);
                vkTools::BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, computeCommandBuffer);
                if (totalTime > SKIP_TIME_NANO) gpuComputeTimer.Start(computeCommandBuffer);

                camera.Update(20.f, 2.f, dt, &inputManager);
                particleUpdateSystem.Update(computeCommandBuffer, &scene, dt);

                if (totalTime > SKIP_TIME_NANO) gpuComputeTimer.Stop(computeCommandBuffer);
                vkTools::EndCommandBuffer(computeCommandBuffer);
                vkTools::QueueSubmit(computeQueue, { computeCommandBuffer }, { computeCompleteSemaphore });
                // SYNC_COMPUTE_GRAPHICS
                if (syncComputeGraphics) vkTools::WaitQueue(computeQueue);
                // --- UPDATE --- //

                // +++ RENDER +++ //
                //vkTools::WaitQueue(graphicsQueue);
                vkTools::ResetCommandBuffer(graphicsCommandBuffer);
                vkTools::BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, graphicsCommandBuffer);
                if (totalTime > SKIP_TIME_NANO) gpuGraphicsTimer.Start(graphicsCommandBuffer);

                camera.mpFrameBuffer->Clear(graphicsCommandBuffer, 0.2f, 0.2f, 0.2f);
                particleRenderSystem.Render(graphicsCommandBuffer, &scene, &camera);

                if (totalTime > SKIP_TIME_NANO) gpuGraphicsTimer.Stop(graphicsCommandBuffer);
                vkTools::EndCommandBuffer(graphicsCommandBuffer);
                vkTools::QueueSubmit(graphicsQueue, { graphicsCommandBuffer }, { graphicsCompleteSemaphore });
                // --- RENDER --- //

                // Wait on CPU for compute and graphics to complete.
                vkTools::WaitQueue(computeQueue);
                vkTools::WaitQueue(graphicsQueue);
            }

            // +++ PRESENET +++ //
            // Wait for frame to complete.
            vkTools::QueueSubmit(graphicsQueue, {}, {}, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { computeCompleteSemaphore, graphicsCompleteSemaphore });

            // Present frame.
            renderer.Present(camera.mpFrameBuffer);
            // --- PRESENET --- //

            // +++ PROFILING +++ //
            if (totalTime > SKIP_TIME_NANO)
            {
                if (frameCount == 0)
                    std::cout << "--- Skip time over --- " << std::endl << std::endl;

                totalMeasureTime += mt;
                ++frameCount;

                float computeTime = 1.f / 1000000.f * gpuComputeTimer.GetDeltaTime();
                float graphicsTime = 1.f / 1000000.f * gpuGraphicsTimer.GetDeltaTime();
                VkCommandBuffer resetTimerCommandBuffer = vkTools::BeginSingleTimeCommand(device, renderer.mGraphicsCommandPool);
                gpuComputeTimer.Reset(resetTimerCommandBuffer);
                gpuGraphicsTimer.Reset(resetTimerCommandBuffer);
                vkTools::EndSingleTimeCommand(device, renderer.mGraphicsCommandPool, renderer.mGraphicsQueue, resetTimerCommandBuffer);

                if (inputManager.KeyPressed(GLFW_KEY_F2))
                {
                    std::cout << "GPU(Total) : " << computeTime + graphicsTime << " ms | GPU(Compute): " << computeTime << " ms | GPU(Graphics) : " << graphicsTime << " ms" << std::endl;
                    profiler.Rectangle(gpuComputeTimer.GetBeginTime(), 1, gpuComputeTimer.GetDeltaTime(), 1, 0.f, 0.f, 1.f);
                    profiler.Rectangle(gpuGraphicsTimer.GetBeginTime(), 0, gpuGraphicsTimer.GetDeltaTime(), 1, 0.f, 1.f, 0.f);
                    profiler.Point(gpuGraphicsTimer.GetBeginTime(), totalMeasureTime / frameCount, syncComputeGraphics ? "'-ro'" : "'-bo'");
                }

                // CALCULATE AVERAGE FRAME TIME OF LAST NUMBER OF FRAMES
                averageTime -= profileFrames[frameCount % PROFILE_FRAME_COUNT];
                profileFrames[frameCount % PROFILE_FRAME_COUNT] = mt;
                averageTime += mt;

                if (inputManager.KeyPressed(GLFW_KEY_F3))
                {
                    std::cout << "CPU(Average delta time of last " << PROFILE_FRAME_COUNT << " frames) : " << averageTime / PROFILE_FRAME_COUNT / 1000000 << " ms : FrameCount: " << frameCount << std::endl;
                }
            }
            // --- PROFILING --- //
        }
    }
    // --- MAIN LOOP --- //

    // +++ SHUTDOWN +++ //
    vkTools::WaitQueue(renderer.mPresentQueue);
    vkTools::WaitQueue(graphicsQueue);
    vkTools::WaitQueue(computeQueue);
    vkTools::FreeCommandBuffer(device, graphicsCommandPool, graphicsCommandBuffer);
    vkTools::FreeCommandBuffer(device, computeCommandPool, computeCommandBuffer);
    vkDestroySemaphore(device, graphicsCompleteSemaphore, nullptr);
    vkDestroySemaphore(device, computeCompleteSemaphore, nullptr);
    // --- SHUTDOWN --- //

    return 0;
}
