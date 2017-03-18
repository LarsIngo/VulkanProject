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

        // Update particles.
        // commandBuffer Command buffer to update.
        // scene Scene to update.
        // dt Delta time.
        //void Update(VkCommandBuffer commandBuffer, Scene* scene, float dt);

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

        //VkShaderModule mComputeShaderModule;


        VkShaderModule mVertexShaderModule;
        VkShaderModule mGeometryShaderModule;
        VkShaderModule mPixelShaderModule;

        VkDescriptorPool mPipelineDescriptorPool;
        VkDescriptorSet mPipelineDescriptorSet;
        VkDescriptorSetLayout mPipelineDescriptorSetLayout;
        VkPipelineLayout mPipelineLayout;
        VkPipeline mPipeline;

        //ID3D11BlendState* mBlendState;
        
        struct UpdateMetaData
        {
            float dt;
            unsigned int particleCount;
            float pad[6];
        } mUpdateMetaData;
        VkBuffer mUpdateMetaDataBuffer;
        VkDeviceMemory mUpdateMetaDataBufferMemory;

        struct RenderMetaData
        {
            glm::mat4 vpMatrix;
            glm::vec3 lensPosition;
            glm::vec3 lensUpDirection;
            float pad[2];
        } mRenderMetaData;
        VkBuffer mRenderMetaDataBuffer;
        VkDeviceMemory mRenderMetaDataBufferMemory;
};
