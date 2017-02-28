#include "VkRenderer.hpp"
#include <assert.h>

VkRenderer::VkRenderer(unsigned int winWidth, unsigned int winHeight)
{
    mWinWidth = winWidth;
    mWinHeight = winHeight;
    mClose = false;

    // Window.
    InitialiseGLFW();

    // Vulkan.
    InitialiseVulkan();
}

VkRenderer::~VkRenderer()
{
    DeInitialiseVulkan();
    DeInitialiseGLFW();
}

bool VkRenderer::Running() const
{
    if (mClose)
        return true;

    glfwPollEvents();

    return !glfwWindowShouldClose(mWindow);
}

void VkRenderer::Close()
{
    mClose = true;
}

void VkRenderer::InitialiseGLFW()
{
    /* Initialize the library */
    if (!glfwInit())
        assert(0 && "GLFWERROR: Initialize the library.");

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    /* Create window */
    mWindow = glfwCreateWindow(mWinWidth, mWinHeight, "Vulkan window", NULL, NULL);
}

void VkRenderer::DeInitialiseGLFW()
{
    glfwTerminate();
}

void VkRenderer::InitialiseVulkan()
{

}

void VkRenderer::DeInitialiseVulkan()
{

}
