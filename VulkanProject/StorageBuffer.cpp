#include "StorageBuffer.hpp"
#include "vkTools.hpp"

StorageBuffer::StorageBuffer(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int totalSize, unsigned int stride)
{
    mDevice = device;
    mPhysicalDevice = physicalDevice;
    mSize = totalSize;
    mStride = stride;
    
    // Storage buffer.
    uint32_t minOffsetAligment;
    vkTools::CreateBuffer(mDevice, mPhysicalDevice, mSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mBuffer, mBufferMemory, minOffsetAligment
    );
    MsgAssert(mStride % minOffsetAligment, 0, "Vulkan runtime error. VKERROR: Unsupported storage buffer aligment.");

    // Staging buffer.
    vkTools::CreateBuffer(mDevice, mPhysicalDevice, mSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mStagingBuffer, mStagingBufferMemory, minOffsetAligment
    );
    MsgAssert(mStride % minOffsetAligment, 0, "Vulkan runtime error. VKERROR: Unsupported storage buffer aligment.");
}

StorageBuffer::~StorageBuffer()
{
    vkFreeMemory(mDevice, mBufferMemory, nullptr);
    vkFreeMemory(mDevice, mStagingBufferMemory, nullptr);
    vkDestroyBuffer(mDevice, mBuffer, nullptr);
    vkDestroyBuffer(mDevice, mStagingBuffer, nullptr);
}

void StorageBuffer::Copy(StorageBuffer* storageBuffer)
{
    assert(storageBuffer != this);
    assert(mSize == storageBuffer->GetSize());

    //mpDeviceContext->CopyResource(mBuff, storageBuffer->mBuff);
}

unsigned int StorageBuffer::GetSize()
{
    return mSize;
}

unsigned int StorageBuffer::GetStride()
{
    return mStride;
}

void StorageBuffer::Write(void* data, unsigned int size, unsigned int offset)
{
    assert(offset + size <= mSize);

    /*D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    DxAssert(mpDeviceContext->Map(mStagingBuff, 0, D3D11_MAP_WRITE, 0, &mappedResource), S_OK);
    memcpy(mappedResource.pData, reinterpret_cast<unsigned char*>(data) + offset, size);
    mpDeviceContext->Unmap(mStagingBuff, 0);

    mpDeviceContext->CopyResource(mBuff, mStagingBuff);*/
}
