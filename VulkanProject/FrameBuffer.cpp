#include "FrameBuffer.hpp"
#include "vkTools.hpp"
#include <assert.h>

FrameBuffer::FrameBuffer(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int width, unsigned int height, VkFormat format, VkRenderPass renderPass, VkImage initImage)
{
    mDevice = device;
    mPhysicalDevice = physicalDevice;
    mWidth = width;
    mHeight = height;

    mImageMemory = VK_NULL_HANDLE;
    mImage = initImage;
    mImageView = VK_NULL_HANDLE;
    mFormat = format;
    mImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    mRenderPass = renderPass;

    mMyImage = initImage == VK_NULL_HANDLE;
    if (mMyImage)
    {   // Allocate device memory and create image.
        vkTools::CreateImage(mDevice, mPhysicalDevice, mWidth, mHeight,
            mFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);
    }

    vkTools::CreateImageView(mDevice, mImage, mFormat, VK_IMAGE_ASPECT_COLOR_BIT, mImageView);

    VkExtent2D extent = { width, height };
    vkTools::CreateFramebuffer(mDevice, extent, mRenderPass, mImageView, VK_NULL_HANDLE, mFrameBuffer);
}

FrameBuffer::~FrameBuffer()
{
    if (mMyImage)
    {
        vkFreeMemory(mDevice, mImageMemory, nullptr);
        vkDestroyImage(mDevice, mImage, nullptr);
    }
    vkDestroyImageView(mDevice, mImageView, nullptr);
    vkDestroyFramebuffer(mDevice, mFrameBuffer, nullptr);
}

void FrameBuffer::Clear(VkCommandBuffer commandBuffer, float r, float g, float b, float a, float depth)
{
    TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkClearColorValue clearColorValue;
    clearColorValue.float32[0] = r;
    clearColorValue.float32[1] = g;
    clearColorValue.float32[2] = b;
    clearColorValue.float32[3] = a;

    VkImageSubresourceRange imageSubresourceRange;
    imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresourceRange.baseMipLevel = 0;
    imageSubresourceRange.levelCount = 1;
    imageSubresourceRange.baseArrayLayer = 0;
    imageSubresourceRange.layerCount = 1;
    
    vkCmdClearColorImage(commandBuffer, mImage, mImageLayout, &clearColorValue, 1, &imageSubresourceRange);
}

void FrameBuffer::Copy(VkCommandBuffer commandBuffer, FrameBuffer* fb)
{
    assert(fb != this);
	assert(mWidth == fb->mWidth && mHeight == fb->mHeight);

    fb->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkTools::CopyImage(commandBuffer, fb->mImage, mImage, mWidth, mHeight);
}

void FrameBuffer::TransitionImageLayout(const VkCommandBuffer& commandBuffer, VkImageLayout newLayout)
{
    if (mImageLayout == newLayout)
        return;

    vkTools::TransitionImageLayout(commandBuffer, mImage, mFormat, mImageLayout, newLayout);
    mImageLayout = newLayout;
}
