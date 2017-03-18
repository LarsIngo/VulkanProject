#pragma once

#include <vulkan/vulkan.h>

class FrameBuffer
{
    public:
        // Constructor.
        // device Vulkan device.
        // physicalDevice Vulkan physical device.
        // width Width in pixels.
        // height Height in pixels.
        // initTexture Initialised image. DEFAULT [VK_NULL_HANDLE]
        FrameBuffer(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int width, unsigned int height, VkFormat format, VkImage initTexture = VK_NULL_HANDLE);

        // Destructor.
        ~FrameBuffer();

        // Clear image.
        void Clear(VkCommandBuffer commandBuffer, float r = 0.f, float g = 0.f, float b = 0.f, float a = 0.f, float depth = 1.f);

		// Copy other frame buffer.
		void Copy(VkCommandBuffer commandBuffer, FrameBuffer* fb);

        // Transition image layout.
        // commandbuffer Command buffer to make transition
        // newLayout Layout to transition to.
        void TransitionImageLayout(const VkCommandBuffer& commandBuffer, VkImageLayout newLayout);

        // Frame buffer width in pixels.
        unsigned int mWidth;
        // Frame buffer height in pixels.
        unsigned int mHeight;

        // Color.
        VkImage mImage;
        VkImageView mImageView;
        VkFormat mFormat;
        VkImageLayout mImageLayout; 
        VkDeviceMemory mDeviceMemory;

    private:
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;
        bool mMyImage;
};
