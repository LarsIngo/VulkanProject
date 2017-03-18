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
}

void VkRenderer::DeInitialiseVulkan()
{
    vkDeviceWaitIdle(mDevice);

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

    float queuePriorities{ 1.f };
    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.queueFamilyIndex = mGraphicsFamilyIndex;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriorities;
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

    vkGetDeviceQueue(mDevice, mGraphicsFamilyIndex, 0, &mGraphicsQueue);
}

void VkRenderer::DeInitialiseDevice()
{
    vkDestroyDevice(mDevice, nullptr);
}

