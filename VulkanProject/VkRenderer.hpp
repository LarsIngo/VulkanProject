#pragma once

#ifdef _DEBUG
#define BUILD_ENABLE_VULKAN_DEBUG
#endif

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <map>
class FrameBuffer;

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

        // Present frame buffer to screen.
        void Present(FrameBuffer* fb);

        // GLFW window.
        GLFWwindow* mGLFWwindow;

        // Vulkan.
        VkInstance mInstance;
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;
        VkPhysicalDeviceProperties mPhysicalDeviceProperties;
        VkPhysicalDeviceFeatures mPhysicalDeviceFeatures;

        uint32_t mPresentFamilyIndex;
        VkQueue mPresentQueue;
        VkCommandPool mPresentCommandPool;
        VkCommandBuffer mPresentCommandBuffer;

        uint32_t mGraphicsFamilyIndex;
        VkCommandPool mGraphicsCommandPool;
        VkQueue mGraphicsQueue;

        uint32_t mComputeFamilyIndex;
        VkCommandPool mComputeCommandPool;
        VkQueue mComputeQueue;

        uint32_t mTransferFamilyIndex;
        VkCommandPool mTransferCommandPool;
        VkQueue mTransferQueue;

        VkSurfaceKHR mSurfaceKHR;
        VkSurfaceFormatKHR mSurfaceFormatKHR;
        VkSurfaceCapabilitiesKHR mSurfaceCapabilitiesKHR;
        VkExtent2D mSurfaceExtent;

        VkSwapchainKHR mSwapchainKHR;
        std::vector<FrameBuffer*> mSwapchainFrameBufferList;

        VkSemaphore mCopyCompleteSemaphore;
        VkSemaphore mPresentCompleteSemaphore;

        VkRenderPass mRenderPass;

    private:
        void InitialiseGLFW();
        void DeInitialiseGLFW();

        void InitialiseVulkan();
        void DeInitialiseVulkan();

        void InitialiseInstance();
        void DeInitialiseInstance();

        void InitialiseSurfaceKHR();
        void DeInitialiseSurfaceKHR();

        void InitialiseDevice();
        void DeInitialiseDevice();

        void InitialiseQueues();
        void DeInitialiseQueues();

        void InitialiseSwapchainKHR();
        void DeInitialiseSwapchainKHR();

        void InitialiseCommandPool();
        void DeInitialiseCommandPool();

        void InitialiseSemaphores();
        void DeInitialiseSemaphores();

        void InitialiseSwapchanFrameBuffers();
        void DeInitialiseSwapchanFrameBuffers();
        
#ifdef BUILD_ENABLE_VULKAN_DEBUG
        VkDebugReportCallbackEXT mDebugReportCallbackEXT;
        PFN_vkCreateDebugReportCallbackEXT mPFNCreateDebugReportCallbackEXT;
        PFN_vkDestroyDebugReportCallbackEXT mPFNDestroyDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT mPFNDebugReportMessageEXT;
#endif

        unsigned int mWinWidth;
        unsigned int mWinHeight;
        bool mClose;
        uint32_t mActiveSwapchainImageIndex;
        std::map<uint32_t, uint32_t> mFamilyQueueCountMap;
};
