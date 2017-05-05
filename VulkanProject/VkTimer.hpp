#pragma once

#include <vulkan/vulkan.h>
#include <assert.h>
#include <vector>

// Vulkan timer.
class VkTimer {
    public:
        // Constructor.
        VkTimer(VkDevice device, VkPhysicalDevice physicalDevice)
        {
            mDevice = device;
            mPhysicalDevice = physicalDevice;
            mActive = false;
            mAccurateTime = false;
            mReset = true;
            mDeltaTime = 0;
            mBeginTime = 0;

            uint32_t queueFamilyPropertyCount;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilyPropertyList;
            queueFamilyPropertyList.resize(queueFamilyPropertyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyPropertyList.data());
            for(auto it : queueFamilyPropertyList)
                assert(it.timestampValidBits == 64); // Use VK_QUERY_RESULT_64_BIT

            VkQueryPoolCreateInfo queryPoolCreateInfo;
            queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            queryPoolCreateInfo.pNext = NULL;
            queryPoolCreateInfo.flags = 0;
            queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolCreateInfo.queryCount = 1;
            queryPoolCreateInfo.pipelineStatistics = NULL;
            vkCreateQueryPool(mDevice, &queryPoolCreateInfo, nullptr, &mStartQuery);
            vkCreateQueryPool(mDevice, &queryPoolCreateInfo, nullptr, &mStopQuery);
        }

        // Destructor.
        ~VkTimer()
        {
            vkDestroyQueryPool(mDevice, mStartQuery, nullptr);
            vkDestroyQueryPool(mDevice, mStopQuery, nullptr);
        }

        // Start timestamp.
        void Start(VkCommandBuffer commandBuffer)
        {
            assert(!mActive && mReset);
            mActive = true;
            mReset = false;
            mAccurateTime = false;

            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mStartQuery, 0);
        }

        // Stop timestamp.
        void Stop(VkCommandBuffer commandBuffer)
        {
            assert(mActive);
            mActive = false;

            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mStopQuery, 0);
        }

        // Get time from start to stop in nano seconds.
        uint64_t GetDeltaTime()
        {
            CalculateTime();
            return mDeltaTime;
        }

        uint64_t GetBeginTime()
        {
            CalculateTime();
            return mBeginTime;
        }

        // Time in nano secounds.
        void CalculateTime()
        {
            if (mAccurateTime) return;
            mAccurateTime = true;

            uint64_t startTime[2];
            VkResult startResult = vkGetQueryPoolResults(mDevice, mStartQuery, 0, 1, sizeof(uint64_t) * 2, &startTime, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_WAIT_BIT);
            assert(startResult == VK_SUCCESS);

            uint64_t stopTime[2];
            VkResult stopResult = vkGetQueryPoolResults(mDevice, mStopQuery, 0, 1, sizeof(uint64_t) * 2, &stopTime, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_WAIT_BIT);
            assert(stopResult == VK_SUCCESS);

            assert(startTime[1] == 1 && stopTime[1] == 1);

            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(mPhysicalDevice, &physicalDeviceProperties);
            mDeltaTime = (stopTime[0] - startTime[0]) * static_cast<double>(physicalDeviceProperties.limits.timestampPeriod);
            mBeginTime = startTime[0] * static_cast<double>(physicalDeviceProperties.limits.timestampPeriod);
        }


        // Whether timer is active.
        bool IsActive()
        {
            return mActive;
        }

        // Resert timer.
        void Reset(VkCommandBuffer commandBuffer)
        {
            assert(!mActive && !mReset);
            mReset = true;

            vkCmdResetQueryPool(commandBuffer, mStartQuery, 0, 1);
            vkCmdResetQueryPool(commandBuffer, mStopQuery, 0, 1);
        }

    private:
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;
        VkQueryPool mStartQuery;
        VkQueryPool mStopQuery;
        bool mActive;
        bool mAccurateTime;
        bool mReset;
        uint64_t mDeltaTime;
        uint64_t mBeginTime;
};
