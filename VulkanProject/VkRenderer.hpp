#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class VkRenderer
{
    public:
        // Constructor.
        // winWidth Window width in pixels.
        // winHeight Window height in pixels.
        VkRenderer(unsigned int winWidth = 640, unsigned int winHeight = 640);

        // Destructor.
        ~VkRenderer();

        // Whether window is running of not.
        bool Running() const;

        // Close window.
        void Close();

    private:
        void InitialiseGLFW();
        void DeInitialiseGLFW();

        void InitialiseVulkan();
        void DeInitialiseVulkan();

        unsigned int mWinWidth;
        unsigned int mWinHeight;
        bool mClose;
        GLFWwindow* mWindow;
};
