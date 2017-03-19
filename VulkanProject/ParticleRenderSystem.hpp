#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class Scene;
class StorageBuffer;
class FrameBuffer;
class Camera;

class ParticleRenderSystem
{
    public:
        // Constructor.
        // device Vulkan device.
        // physicalDevice Vulkan physical device.
        ParticleRenderSystem(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int width, unsigned int height, VkFormat format, VkRenderPass renderPass);

        // Destructor.
        ~ParticleRenderSystem();

        // Render particles.
        // commandBuffer Command buffer to render.
        // scene Scene to render.
        // camera Camera to render from.
        void Render(VkCommandBuffer commandBuffer, Scene* scene, Camera* camera);

    private:
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;
        VkExtent2D mExtent;
        VkFormat mFormat;
        VkRenderPass mRenderPass;

        VkShaderModule mVertexShaderModule;
        VkShaderModule mGeometryShaderModule;
        VkShaderModule mPixelShaderModule;

        VkDescriptorPool mPipelineDescriptorPool;
        VkDescriptorSet mPipelineDescriptorSet;
        VkDescriptorSetLayout mPipelineDescriptorSetLayout;
        VkPipelineLayout mPipelineLayout;
        VkPipeline mPipeline;

        struct MetaData
        {
            glm::mat4 vpMatrix;
            glm::vec4 lensPosition;
            glm::vec4 lensUpDirection;
        } mMetaData;
        VkBuffer mMetaDataBuffer;
        VkDeviceMemory mMetaDataBufferMemory;
};
