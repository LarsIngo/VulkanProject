#pragma once

#include <vulkan/vulkan.h>

class StorageBuffer
{
    public:
        // Constructor.
        // device Vulkan device.
        // physicalDevice Vulkan physical device.
        // totalSize Total size in bytes.
        // stride Stride of each element in bytes.
        StorageBuffer(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int totalSize, unsigned int stride);

        // Destructor.
        ~StorageBuffer();

        // Copy other storage buffer.
        // commandBuffer Command buffer to make copy.
        void Copy(VkCommandBuffer commandBuffer, StorageBuffer* storageBuffer);

        // Get size of storage buffer.
        // Returns size in bytes.
        unsigned int GetSize();

        // Get stride of storage buffer.
        // Returns stride of each element in bytes.
        unsigned int GetStride();
        
        // Write to storage buffer.
        // commandBuffer Command buffer to make write.
        // data Data to write.
        // byteSize Size of data in bytes.
        // off Offset to write data in bytes.
        void Write(VkCommandBuffer commandBuffer, void* data, unsigned int byteSize, unsigned int offset);

        // Storage buffer.
        VkBuffer mBuffer;
        VkDeviceMemory mBufferMemory;

    private:
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;

        // Staging buffer.
        VkBuffer mStagingBuffer;
        VkDeviceMemory mStagingBufferMemory;

        // Storage buffer stride of each element in bytes.
        unsigned int mStride;

        // Storage buffer size in bytes.
        unsigned int mSize;
};
