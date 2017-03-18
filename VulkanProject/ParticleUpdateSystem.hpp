#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class Scene;
class StorageBuffer;
class FrameBuffer;
class Camera;

class ParticleUpdateSystem
{
    public:
        // Constructor.
        // device Vulkan device.
        // physicalDevice Vulkan physical device.
        ParticleUpdateSystem(VkDevice device, VkPhysicalDevice physicalDevice);

        // Destructor.
        ~ParticleUpdateSystem();

        // Update particles.
        // commandBuffer Command buffer to update.
        // scene Scene to update.
        // dt Delta time.
        void Update(VkCommandBuffer commandBuffer, Scene* scene, float dt);

    private:
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;

        VkShaderModule mComputeShaderModule;

        //VkDescriptorPool mPipelineDescriptorPool;
        //VkDescriptorSet mPipelineDescriptorSet;
        //VkDescriptorSetLayout mPipelineDescriptorSetLayout;
        //VkPipelineLayout mPipelineLayout;
        //VkPipeline mPipeline;
        
        struct MetaData
        {
            float dt;
            unsigned int particleCount;
            float pad[6];
        } mMetaData;
        VkBuffer mMetaDataBuffer;
        VkDeviceMemory mMetaDataBufferMemory;
};
