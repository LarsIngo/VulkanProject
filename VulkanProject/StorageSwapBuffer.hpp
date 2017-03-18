#pragma once

#include "StorageBuffer.hpp"

class StorageSwapBuffer
{
    public:
        // Constructor.
        // device Vulkan device.
        // physicalDevice Vulkan physical device.
        // totalSize Total size in bytes.
        // stride Stride of each element in bytes.
        StorageSwapBuffer(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int totalSize, unsigned int stride);

        // Destructor.
        ~StorageSwapBuffer();

        // Get write storage buffer.
        // Returns output storage buffer.
        StorageBuffer* GetOutputBuffer();

        // Get read storage buffer.
        // Returns input storage buffer.
        StorageBuffer* GetInputBuffer();

        // Swaps read and write storage buffer. 
        void Swap();

    private:
        // Storage buffer count.
        static const unsigned int mBufferCount = 2;
        // Storeage buffers.
        StorageBuffer* mBuffers[mBufferCount];
        // Buffer index.
        unsigned int mBufferIndex = 0;
};
