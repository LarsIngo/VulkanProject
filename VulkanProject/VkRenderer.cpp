#include "VkRenderer.hpp"
#include "vkTools.hpp"

#include <assert.h>
#include <iostream>
#include <sstream>
#include <Windows.h>

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

    return !glfwWindowShouldClose(mGLFWwindow);
}

void VkRenderer::Close()
{
    mClose = true;
}

FrameBuffer* VkRenderer::SwapBackBuffer()
{
    vkTools::VkErrorCheck(vkAcquireNextImageKHR(mDevice, mSwapchainKHR, (std::numeric_limits<uint64_t>::max)(), mPresentCompleteSemaphore, VK_NULL_HANDLE, &mActiveSwapchainImageIndex));
    assert(mActiveSwapchainImageIndex <= mSwapchainFrameBufferList.size());

    return mSwapchainFrameBufferList[mActiveSwapchainImageIndex];
}

void VkRenderer::PresentBackBuffer()
{
    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.pWaitDstStageMask = 0;
    submitInfo.commandBufferCount = 0;
    submitInfo.waitSemaphoreCount = 0;
    VkSemaphore signalSemaphores[] = { mGraphicsCompleteSemaphore };
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.signalSemaphoreCount = 1;

    vkTools::VkErrorCheck(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

    VkPresentInfoKHR presentInfoKHR;
    presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfoKHR.pNext = NULL;
    presentInfoKHR.pResults = NULL;
    VkSemaphore waitSemaphores[] = { mPresentCompleteSemaphore, mGraphicsCompleteSemaphore };
    presentInfoKHR.pWaitSemaphores = waitSemaphores;
    presentInfoKHR.waitSemaphoreCount = 2;
    VkSwapchainKHR swapchains[] = { mSwapchainKHR };
    presentInfoKHR.pSwapchains = swapchains;
    presentInfoKHR.swapchainCount = 1;
    presentInfoKHR.pImageIndices = &mActiveSwapchainImageIndex;
    vkQueuePresentKHR(mPresentQueue, &presentInfoKHR);
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
    mGLFWwindow = glfwCreateWindow(mWinWidth, mWinHeight, "Vulkan window", NULL, NULL);
}

void VkRenderer::DeInitialiseGLFW()
{
    glfwTerminate();
}

void VkRenderer::InitialiseVulkan()
{
    InitialiseInstance();
    InitialiseDevice();
    InitialiseSurfaceKHR();
    InitialiseQueues();
    InitialiseSwapchainKHR();
    InitialiseCommandPool();
    InitialiseSemaphores();
    InitialiseSwapchanFrameBuffers();
}

void VkRenderer::DeInitialiseVulkan()
{
    vkDeviceWaitIdle(mDevice);

    DeInitialiseSwapchanFrameBuffers();
    DeInitialiseSemaphores();
    DeInitialiseCommandPool();
    DeInitialiseSwapchainKHR();
    DeInitialiseQueues();
    DeInitialiseSurfaceKHR();
    DeInitialiseDevice();
    DeInitialiseInstance();
}

#ifdef BUILD_ENABLE_VULKAN_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugReportCallbackFunction(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  obj_type,
    uint64_t                    src_obj,
    size_t                      location,
    int32_t                     msg_code,
    const char*                 layer_prefix,
    const char*                 msg,
    void*                       user_data
)
{
    std::ostringstream stream;
    stream << "VKDBG: ";
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        stream << "INFO";
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        stream << "WARNING";
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        stream << "PERFORMANCE_WARNING";
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        stream << "ERROR";
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        stream << "DEBUG";

    stream << "@[" << layer_prefix << "]: ";
    stream << "Code: " << msg_code << " : " << msg << std::endl;

    std::cout << stream.str();

#ifdef _WIN32
    std::string str = stream.str();
    //std::wstring wStr(str.begin(), str.end());
    MessageBox(NULL, str.c_str(), "Vulkan Error!", 0);
#endif
    return VK_FALSE;
}
#endif // BUILD_ENABLE_VULKAN_DEBUG

void VkRenderer::InitialiseInstance()
{
    std::vector<const char*> enabledExtensionList;
    std::vector<const char*> enabledLayerList;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (unsigned int i = 0; i < glfwExtensionCount; i++)
        enabledExtensionList.push_back(glfwExtensions[i]);
#ifdef BUILD_ENABLE_VULKAN_DEBUG
    VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfoEXT;
    enabledLayerList.push_back("VK_LAYER_LUNARG_standard_validation");
    enabledExtensionList.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    debugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugReportCallbackCreateInfoEXT.pUserData = NULL;
    debugReportCallbackCreateInfoEXT.pfnCallback = (PFN_vkDebugReportCallbackEXT)VulkanDebugReportCallbackFunction;
    debugReportCallbackCreateInfoEXT.pNext = NULL;
    debugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
#endif

    VkApplicationInfo applicationInfo;
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);
    applicationInfo.pApplicationName = "Vulkan Renderer";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pNext = NULL;

    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(enabledExtensionList.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensionList.data();
    instanceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(enabledLayerList.size());
    instanceCreateInfo.ppEnabledLayerNames = enabledLayerList.data();
    instanceCreateInfo.flags = 0;
#ifdef BUILD_ENABLE_VULKAN_DEBUG
    instanceCreateInfo.pNext = &debugReportCallbackCreateInfoEXT;
#else
    instanceCreateInfo.pNext = NULL;
#endif
    vkTools::VkErrorCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance));

#ifdef BUILD_ENABLE_VULKAN_DEBUG
    mPFNCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT"));
    mPFNDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(mInstance, "vkDestroyDebugReportCallbackEXT"));
    mPFNDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(mInstance, "vkDebugReportMessageEXT"));
    if (mPFNCreateDebugReportCallbackEXT == VK_NULL_HANDLE || mPFNDestroyDebugReportCallbackEXT == VK_NULL_HANDLE)
    {
        assert(0 && "VKERROR: Could not fetch debug function pointers.");
        std::exit(-1);
    }
    vkTools::VkErrorCheck(mPFNCreateDebugReportCallbackEXT(mInstance, &debugReportCallbackCreateInfoEXT, nullptr, &mDebugReportCallbackEXT));
#endif
}

void VkRenderer::DeInitialiseInstance()
{
#ifdef BUILD_ENABLE_VULKAN_DEBUG
    mPFNDestroyDebugReportCallbackEXT(mInstance, mDebugReportCallbackEXT, nullptr);
#endif
    vkDestroyInstance(mInstance, nullptr);
}

void VkRenderer::InitialiseSurfaceKHR()
{
    vkTools::VkErrorCheck(glfwCreateWindowSurface(mInstance, mGLFWwindow, nullptr, &mSurfaceKHR));

    uint32_t surfaceCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurfaceKHR, &surfaceCount, nullptr);
    if (surfaceCount == 0) {
        assert(0 && "VULKANERROR: Surface format missing.");
        std::exit(-1);
    }
    std::vector<VkSurfaceFormatKHR> surfaceFormatKHRList(surfaceCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurfaceKHR, &surfaceCount, surfaceFormatKHRList.data());
    if (surfaceFormatKHRList[0].format == VK_FORMAT_UNDEFINED) {
        mSurfaceFormatKHR.format = VK_FORMAT_B8G8R8A8_UNORM;
        mSurfaceFormatKHR.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    }
    else {
        mSurfaceFormatKHR = surfaceFormatKHRList[0];
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurfaceKHR, &mSurfaceCapabilitiesKHR);
    if (mSurfaceCapabilitiesKHR.currentExtent.width < UINT32_MAX) {
        mSurfaceExtent.width = mSurfaceCapabilitiesKHR.currentExtent.width;
        mSurfaceExtent.height = mSurfaceCapabilitiesKHR.currentExtent.height;
    }
}

void VkRenderer::DeInitialiseSurfaceKHR()
{
    vkDestroySurfaceKHR(mInstance, mSurfaceKHR, nullptr);
}

void VkRenderer::InitialiseDevice()
{
    {
        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDeviceList(physicalDeviceCount);
        vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDeviceList.data());
        mPhysicalDevice = physicalDeviceList[0];
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &mPhysicalDeviceProperties);
        vkGetPhysicalDeviceFeatures(mPhysicalDevice, &mPhysicalDeviceFeatures);
    }

    std::vector<const char*> enabledExtensionList = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    mGraphicsFamilyIndex = vkTools::FindGraphicsFamilyIndex(mPhysicalDevice);
    mComputeFamilyIndex = vkTools::FindComputeFamilyIndex(mPhysicalDevice);
    assert(mGraphicsFamilyIndex == mComputeFamilyIndex);

    float queuePriorities{ 1.f };
    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriorities;
    deviceQueueCreateInfo.queueFamilyIndex = mGraphicsFamilyIndex;
    deviceQueueCreateInfo.pNext = NULL;
    deviceQueueCreateInfo.flags = 0;

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledLayerCount = NULL;
    deviceCreateInfo.ppEnabledLayerNames = NULL;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionList.size());
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensionList.data();
    deviceCreateInfo.pEnabledFeatures = &mPhysicalDeviceFeatures;
    deviceCreateInfo.pNext = NULL;
    deviceCreateInfo.flags = 0;

    vkTools::VkErrorCheck(vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice));
}

void VkRenderer::DeInitialiseDevice()
{
    vkDestroyDevice(mDevice, nullptr);
}

void VkRenderer::InitialiseQueues()
{
    mPresentFamilyIndex = vkTools::FindPresentFamilyIndex(mPhysicalDevice, mSurfaceKHR);

    vkGetDeviceQueue(mDevice, mGraphicsFamilyIndex, 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, mComputeFamilyIndex, 0, &mComputeQueue);
    vkGetDeviceQueue(mDevice, mPresentFamilyIndex, 0, &mPresentQueue);
}

void VkRenderer::DeInitialiseQueues()
{

}

void VkRenderer::InitialiseSwapchainKHR()
{
    uint32_t swapchainImageCount = 0;
    if (swapchainImageCount < mSurfaceCapabilitiesKHR.minImageCount + 1)
        swapchainImageCount = mSurfaceCapabilitiesKHR.minImageCount + 1;
    if (mSurfaceCapabilitiesKHR.maxImageCount > 0)
        if (swapchainImageCount > mSurfaceCapabilitiesKHR.maxImageCount)
            swapchainImageCount = mSurfaceCapabilitiesKHR.maxImageCount;
    mSwapchainFrameBufferList.resize(swapchainImageCount);

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    {
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurfaceKHR, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModeList(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurfaceKHR, &presentModeCount, presentModeList.data());
        for (auto m : presentModeList)
            if (m == VK_PRESENT_MODE_MAILBOX_KHR)
                present_mode = m;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR;
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.surface = mSurfaceKHR;
    swapchainCreateInfoKHR.minImageCount = swapchainImageCount;
    swapchainCreateInfoKHR.imageFormat = mSurfaceFormatKHR.format;
    swapchainCreateInfoKHR.imageColorSpace = mSurfaceFormatKHR.colorSpace;
    swapchainCreateInfoKHR.imageExtent.width = mSurfaceExtent.width;
    swapchainCreateInfoKHR.imageExtent.height = mSurfaceExtent.height;
    swapchainCreateInfoKHR.imageArrayLayers = 1;
    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; //VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT

    uint32_t queueFamilyIndices[] = { mGraphicsFamilyIndex, mPresentFamilyIndex };
    if (mGraphicsFamilyIndex != mPresentFamilyIndex) {
        swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfoKHR.queueFamilyIndexCount = 2;
        swapchainCreateInfoKHR.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // (SLI), several queue families render to same swap chain OR have one queue render and another present 
        swapchainCreateInfoKHR.queueFamilyIndexCount = 0; // Not used if VK_SHARING_MODE_EXCLUSIVE
        swapchainCreateInfoKHR.pQueueFamilyIndices = nullptr; // Not used if VK_SHARING_MODE_EXCLUSIVE
    }

    swapchainCreateInfoKHR.preTransform = mSurfaceCapabilitiesKHR.currentTransform;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.presentMode = present_mode;
    swapchainCreateInfoKHR.clipped = VK_TRUE;

    swapchainCreateInfoKHR.pNext = 0;
    swapchainCreateInfoKHR.flags = 0;

    VkSwapchainKHR oldSwapchainKHR = mSwapchainKHR;
    swapchainCreateInfoKHR.oldSwapchain = oldSwapchainKHR;

    VkSwapchainKHR newSwapchainKHR = VK_NULL_HANDLE;
    vkTools::VkErrorCheck(vkCreateSwapchainKHR(mDevice, &swapchainCreateInfoKHR, nullptr, &newSwapchainKHR));

    mSwapchainKHR = newSwapchainKHR;

    vkTools::VkErrorCheck(vkGetSwapchainImagesKHR(mDevice, mSwapchainKHR, &swapchainImageCount, nullptr));

    vkGetDeviceQueue(mDevice, mPresentFamilyIndex, 0, &mPresentQueue);
}

void VkRenderer::DeInitialiseSwapchainKHR()
{
    vkDestroySwapchainKHR(mDevice, mSwapchainKHR, nullptr);
}

void VkRenderer::InitialiseCommandPool()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.pNext = NULL;

    commandPoolCreateInfo.queueFamilyIndex = mGraphicsFamilyIndex;
    vkTools::VkErrorCheck(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mGraphicsCommandPool));

    commandPoolCreateInfo.queueFamilyIndex = mComputeFamilyIndex;
    vkTools::VkErrorCheck(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mComputeCommandPool));
}

void VkRenderer::DeInitialiseCommandPool()
{
    vkDestroyCommandPool(mDevice, mGraphicsCommandPool, nullptr);
    vkDestroyCommandPool(mDevice, mComputeCommandPool, nullptr);
}

void VkRenderer::InitialiseSemaphores()
{
    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = NULL;
    semaphoreCreateInfo.flags = 0;

    vkTools::VkErrorCheck(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mGraphicsCompleteSemaphore));
    vkTools::VkErrorCheck(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mComputeCompleteSemaphore));
    vkTools::VkErrorCheck(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mPresentCompleteSemaphore));
}

void VkRenderer::DeInitialiseSemaphores()
{
    vkDestroySemaphore(mDevice, mGraphicsCompleteSemaphore, nullptr);
    vkDestroySemaphore(mDevice, mComputeCompleteSemaphore, nullptr);
    vkDestroySemaphore(mDevice, mPresentCompleteSemaphore, nullptr);
}

void VkRenderer::InitialiseSwapchanFrameBuffers()
{
    std::vector<VkImage> swapchainImageList;
    swapchainImageList.resize(mSwapchainFrameBufferList.size());

    uint32_t swapchainImageCount = static_cast<uint32_t>(mSwapchainFrameBufferList.size());
    vkTools::VkErrorCheck(vkGetSwapchainImagesKHR(mDevice, mSwapchainKHR, &swapchainImageCount, swapchainImageList.data()));

    for (std::size_t i = 0; i < mSwapchainFrameBufferList.size(); ++i)
        mSwapchainFrameBufferList[i] = new FrameBuffer(mDevice, mPhysicalDevice, mWinWidth, mWinHeight, mSurfaceFormatKHR.format, swapchainImageList[i]);
}

void VkRenderer::DeInitialiseSwapchanFrameBuffers()
{
    for (std::size_t i = 0; i < mSwapchainFrameBufferList.size(); ++i)
        delete mSwapchainFrameBufferList[i];
}

