#include "StorageSwapBuffer.hpp"

StorageSwapBuffer::StorageSwapBuffer(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int totalSize, unsigned int stride)
{
    for (unsigned int i = 0; i < mBufferCount; ++i)
        mBuffers[i] = new StorageBuffer(device, physicalDevice, totalSize, stride);
}

StorageSwapBuffer::~StorageSwapBuffer()
{
    for (unsigned int i = 0; i < mBufferCount; ++i)
        delete mBuffers[i];
}

StorageBuffer* StorageSwapBuffer::GetOutputBuffer()
{
    return mBuffers[mBufferIndex];
}

StorageBuffer* StorageSwapBuffer::GetInputBuffer()
{
    return mBuffers[(mBufferIndex + 1) % mBufferCount];
}

void StorageSwapBuffer::Swap()
{
    mBufferIndex = (mBufferIndex + 1) % mBufferCount;
}
