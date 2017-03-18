#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class Scene;
class StorageBuffer;
class FrameBuffer;
class Camera;

class ParticleSystem
{
    public:
        // Constructor.
        // device Vulkan device.
        // physicalDevice Vulkan physical device.
        ParticleSystem(VkDevice device, VkPhysicalDevice physicalDevice);

        // Destructor.
        ~ParticleSystem();

        // Update particles.
        // commandBuffer Command buffer to update.
        // scene Scene to update.
        // dt Delta time.
        void Update(VkCommandBuffer commandBuffer, Scene* scene, float dt);

        // Render particles.
        // commandBuffer Command buffer to render.
        // scene Scene to render.
        // camera Camera to render from.
        void Render(VkCommandBuffer commandBuffer, Scene* scene, Camera* camera);

    private:
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;

        //VkShaderModule mComputeShaderModule;
        //ID3D11ComputeShader* mComputeShader;

        //ID3D11VertexShader* mVertexShader;
        //ID3D11GeometryShader* mGeometryShader;
        //ID3D11PixelShader* mPixelShader;
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
