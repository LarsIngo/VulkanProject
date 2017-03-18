#pragma once

#define BUILD_ENABLE_VULKAN_DEBUG

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

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

        // GLFW window.
        GLFWwindow* mGLFWwindow;

        // Vulkan.
        VkInstance mInstance;
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;
        VkPhysicalDeviceProperties mPhysicalDeviceProperties;
        VkPhysicalDeviceFeatures mPhysicalDeviceFeatures;

        uint32_t mGraphicsFamilyIndex;
        uint32_t mComputeFamilyIndex;
        uint32_t mPresentFamilyIndex;
        VkQueue mGraphicsQueue;
        VkQueue mComputeQueue;

        VkCommandPool mCommandPool;

        VkSurfaceKHR mSurfaceKHR;
        VkSurfaceCapabilitiesKHR mSurfaceCapabilitiesKHR;
        VkSurfaceFormatKHR mSurfaceFormatKHR;

        VkSwapchainKHR mSwapchainKHR;

        VkSemaphore mPresentCompleteSemaphore;

    private:
        void InitialiseGLFW();
        void DeInitialiseGLFW();

        void InitialiseVulkan();
        void DeInitialiseVulkan();

        void InitialiseInstance();
        void DeInitialiseInstance();

        void InitialiseDevice();
        void DeInitialiseDevice();
        
#ifdef BUILD_ENABLE_VULKAN_DEBUG
        VkDebugReportCallbackEXT mDebugReportCallbackEXT;
        PFN_vkCreateDebugReportCallbackEXT mPFNCreateDebugReportCallbackEXT;
        PFN_vkDestroyDebugReportCallbackEXT mPFNDestroyDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT mPFNDebugReportMessageEXT;
#endif

        unsigned int mWinWidth;
        unsigned int mWinHeight;
        bool mClose;
};
