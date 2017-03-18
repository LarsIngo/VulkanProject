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
        // scene Scene to update.
        // dt Delta time.
        void Update(Scene* scene, float dt);

        // Render particles.
        // scene Scene to render.
        // camera Camera to render from.
        void Render(Scene* scene, Camera* camera);

    private:
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;

        /*ID3D11ComputeShader* mComputeShader;

        ID3D11VertexShader* mVertexShader;
        ID3D11GeometryShader* mGeometryShader;
        ID3D11PixelShader* mPixelShader;
        ID3D11BlendState* mBlendState;
        
        struct UpdateMetaData
        {
            float dt;
            unsigned int particleCount;
            float pad[6];
        } mUpdateMetaData;
        ID3D11ShaderResourceView* mUpdateMetaDataBuffer;

        struct RenderMetaData
        {
            glm::mat4 vpMatrix;
            glm::vec3 lensPosition;
            glm::vec3 lensUpDirection;
            float pad[6];
        } mRenderMetaData;
        ID3D11ShaderResourceView* mRenderMetaDataBuffer;*/
};
