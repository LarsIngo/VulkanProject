#include "StorageBuffer.hpp"
#include "vkTools.hpp"

StorageBuffer::StorageBuffer(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int totalSize, unsigned int stride)
{
    mDevice = device;
    mPhysicalDevice = physicalDevice;
    mSize = totalSize;
    mStride = stride;
    
    // Storage buffer.
    uint32_t minOffsetAlignment;
    vkTools::CreateBuffer(mDevice, mPhysicalDevice, mSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mBuffer, mBufferMemory, minOffsetAlignment
        );
    MsgAssert(mStride % minOffsetAlignment, 0, "Vulkan runtime error. VKERROR: Unsupported storage buffer alignment.");

    // Staging buffer.
    vkTools::CreateBuffer(mDevice, mPhysicalDevice, mSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mStagingBuffer, mStagingBufferMemory, minOffsetAlignment
        );
    MsgAssert(mStride % minOffsetAlignment, 0, "Vulkan runtime error. VKERROR: Unsupported storage buffer alignment.");
}

StorageBuffer::~StorageBuffer()
{
    vkFreeMemory(mDevice, mBufferMemory, nullptr);
    vkFreeMemory(mDevice, mStagingBufferMemory, nullptr);
    vkDestroyBuffer(mDevice, mBuffer, nullptr);
    vkDestroyBuffer(mDevice, mStagingBuffer, nullptr);
}

void StorageBuffer::Copy(VkCommandBuffer commandBuffer, StorageBuffer* storageBuffer)
{
    assert(storageBuffer != this);
    assert(mSize == storageBuffer->GetSize());

    vkTools::CopyBuffer(commandBuffer, storageBuffer->mBuffer, mBuffer, mSize, 0, 0);
}

unsigned int StorageBuffer::GetSize()
{
    return mSize;
}

unsigned int StorageBuffer::GetStride()
{
    return mStride;
}

void StorageBuffer::Write(VkCommandBuffer commandBuffer, void* data, unsigned int byteSize, unsigned int offset)
{
    assert(offset + byteSize <= mSize);

    vkTools::WriteBuffer(commandBuffer, mDevice, mStagingBufferMemory, data, byteSize, offset);

    vkTools::CopyBuffer(commandBuffer, mStagingBuffer, mBuffer, mSize, 0, 0);
}
