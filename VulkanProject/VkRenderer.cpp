#include "VkRenderer.hpp"
#include "vkTools.hpp"

#include <assert.h>
#include <iostream>
#include <sstream>
#include <map>

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
    VkPresentInfoKHR presentInfoKHR;
    presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfoKHR.pNext = NULL;
    presentInfoKHR.pResults = NULL;
    std::vector<VkSemaphore> waitSemaphoresList = { mPresentCompleteSemaphore };
    presentInfoKHR.pWaitSemaphores = waitSemaphoresList.data();
    presentInfoKHR.waitSemaphoreCount = waitSemaphoresList.size();
    VkSwapchainKHR swapchains[] = { mSwapchainKHR };
    presentInfoKHR.pSwapchains = swapchains;
    presentInfoKHR.swapchainCount = 1;
    presentInfoKHR.pImageIndices = &mActiveSwapchainImageIndex;
    vkQueuePresentKHR(mPresentQueue, &presentInfoKHR);

    //vkTools::QueueSubmit(mPresentQueue, {}, {}, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { mPresentCompleteSemaphore });
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


    std::vector<VkQueueFamilyProperties> queue_family_properties_list;
    {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queue_family_count, nullptr);
        queue_family_properties_list.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queue_family_count, queue_family_properties_list.data());
    }

    // https://gist.github.com/sheredom/523f02bbad2ae397d7ed255f3f3b5a7f
    vkTools::PrintFamilyIndices(mPhysicalDevice);

    uint32_t graphicsQueueCount;
    vkTools::FindGraphicsFamily(mPhysicalDevice, mGraphicsFamilyIndex, graphicsQueueCount);
    uint32_t computeQueueCount;
    vkTools::FindComputeFamily(mPhysicalDevice, mComputeFamilyIndex, computeQueueCount);
    uint32_t transferQueueCount;
    vkTools::FindTransferFamily(mPhysicalDevice, mTransferFamilyIndex, transferQueueCount);

    std::cout << "Graphics family index: " << mGraphicsFamilyIndex << std::endl;
    std::cout << "Compute family index: " << mComputeFamilyIndex << std::endl;
    std::cout << "Transfer family index: " << mTransferFamilyIndex << std::endl;

    std::map<uint32_t, uint32_t> familyIndexMap;
    familyIndexMap[mGraphicsFamilyIndex] = mGraphicsFamilyIndex;
    familyIndexMap[mComputeFamilyIndex] = mComputeFamilyIndex;
    familyIndexMap[mTransferFamilyIndex] = mTransferFamilyIndex;

    if (mFamilyQueueCountMap.find(mGraphicsFamilyIndex) == mFamilyQueueCountMap.end())
        mFamilyQueueCountMap[mGraphicsFamilyIndex] = graphicsQueueCount;
    if (mFamilyQueueCountMap.find(mComputeFamilyIndex) == mFamilyQueueCountMap.end())
        mFamilyQueueCountMap[mComputeFamilyIndex] = computeQueueCount;
    if (mFamilyQueueCountMap.find(mTransferFamilyIndex) == mFamilyQueueCountMap.end())
        mFamilyQueueCountMap[mTransferFamilyIndex] = transferQueueCount;
    
    std::vector<VkDeviceQueueCreateInfo> device_queue_create_info;
    std::map<uint32_t, std::vector<float>> queue_Priorities_map;
    for (std::size_t i = 0; i < familyIndexMap.size(); ++i)
    {
        uint32_t familyIndex = familyIndexMap[i];
        uint32_t queueCount = mFamilyQueueCountMap[familyIndex];// familyIndex == 0 ? 16 : 1;
        std::vector<float>& queue_Priorities_list = queue_Priorities_map[familyIndex];
        for (uint32_t n = 0; n < queueCount; ++n)
            queue_Priorities_list.push_back(1.f);

        VkDeviceQueueCreateInfo deviceQueueCreateInfo;
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueCount = queueCount;
        deviceQueueCreateInfo.pQueuePriorities = queue_Priorities_list.data();
        deviceQueueCreateInfo.queueFamilyIndex = familyIndex;
        deviceQueueCreateInfo.pNext = NULL;
        deviceQueueCreateInfo.flags = 0;
        device_queue_create_info.push_back(deviceQueueCreateInfo);
    }

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = device_queue_create_info.size();
    deviceCreateInfo.pQueueCreateInfos = device_queue_create_info.data();
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

    std::map<uint32_t, uint32_t> queueIndexMap;
    queueIndexMap[mPresentFamilyIndex] = 0;
    queueIndexMap[mGraphicsFamilyIndex] = 0;
    queueIndexMap[mComputeFamilyIndex] = 0;
    queueIndexMap[mTransferFamilyIndex] = 0;

    std::cout << std::endl;
    std::cout << "Present queue index: " << queueIndexMap[mPresentFamilyIndex] << std::endl;
    vkGetDeviceQueue(mDevice, mPresentFamilyIndex, queueIndexMap[mPresentFamilyIndex]++, &mPresentQueue);
    std::cout << "Graphics queue index: " << queueIndexMap[mGraphicsFamilyIndex] << std::endl;
    vkGetDeviceQueue(mDevice, mGraphicsFamilyIndex, queueIndexMap[mGraphicsFamilyIndex]++, &mGraphicsQueue);
    std::cout << "Compute queue index: " << queueIndexMap[mComputeFamilyIndex] << std::endl;
    vkGetDeviceQueue(mDevice, mComputeFamilyIndex, queueIndexMap[mComputeFamilyIndex]++, &mComputeQueue);
    std::cout << "Transform queue index: " << queueIndexMap[mTransferFamilyIndex] << std::endl;
    vkGetDeviceQueue(mDevice, mTransferFamilyIndex, queueIndexMap[mTransferFamilyIndex]++, &mTransferQueue);
    std::cout << std::endl;

    assert(queueIndexMap[mPresentFamilyIndex] <= mFamilyQueueCountMap[mPresentFamilyIndex]);
    assert(queueIndexMap[mGraphicsFamilyIndex] <= mFamilyQueueCountMap[mGraphicsFamilyIndex]);
    assert(queueIndexMap[mComputeFamilyIndex] <= mFamilyQueueCountMap[mComputeFamilyIndex]);
    assert(queueIndexMap[mTransferFamilyIndex] <= mFamilyQueueCountMap[mTransferFamilyIndex]);
}

void VkRenderer::DeInitialiseQueues()
{
    vkTools::WaitQueue(mGraphicsQueue);
    vkTools::WaitQueue(mComputeQueue);
    vkTools::WaitQueue(mTransferQueue);
    vkTools::WaitQueue(mPresentQueue);
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

    commandPoolCreateInfo.queueFamilyIndex = mTransferFamilyIndex;
    vkTools::VkErrorCheck(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mTransferCommandPool));
}

void VkRenderer::DeInitialiseCommandPool()
{
    vkDestroyCommandPool(mDevice, mGraphicsCommandPool, nullptr);
    vkDestroyCommandPool(mDevice, mComputeCommandPool, nullptr);
    vkDestroyCommandPool(mDevice, mTransferCommandPool, nullptr);
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

    vkTools::CreateRenderPass(mDevice, mSurfaceFormatKHR.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_FORMAT_UNDEFINED, mRenderPass);
    for (std::size_t i = 0; i < mSwapchainFrameBufferList.size(); ++i)
        mSwapchainFrameBufferList[i] = new FrameBuffer(mDevice, mPhysicalDevice, mWinWidth, mWinHeight, mSurfaceFormatKHR.format, mRenderPass, swapchainImageList[i]);
}

void VkRenderer::DeInitialiseSwapchanFrameBuffers()
{
    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
    for (std::size_t i = 0; i < mSwapchainFrameBufferList.size(); ++i)
        delete mSwapchainFrameBufferList[i];
}

