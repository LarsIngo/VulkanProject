#include "ParticleUpdateSystem.hpp"
#include "VkRenderer.hpp"
#include "Scene.hpp"
#include "FrameBuffer.hpp"
#include "StorageSwapBuffer.hpp"
#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "vkTools.hpp"

ParticleUpdateSystem::ParticleUpdateSystem(VkDevice device, VkPhysicalDevice physicalDevice)
{
    mDevice = device;
    mPhysicalDevice = physicalDevice;

    // Create meta buffers.
    uint32_t minOffsetAligment;
    vkTools::CreateBuffer(mDevice, mPhysicalDevice, sizeof(MetaData),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mMetaDataBuffer, mMetaDataBufferMemory, minOffsetAligment
        );
    assert(sizeof(MetaData) % minOffsetAligment == 0);

    // Create compute pipeline.
    {
        vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Update_CS.spv", mComputeShaderModule);
        /*vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Render_VS.spv", mVertexShaderModule);
        vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Render_GS.spv", mGeometryShaderModule);
        vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Render_PS.spv", mPixelShaderModule);

        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
        descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBinding.binding = 0;
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList{ descriptorSetLayoutBinding };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pNext = 0;
        descriptorSetLayoutCreateInfo.flags = 0;
        descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindingList.size();
        descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindingList.data();
        vkCreateDescriptorSetLayout(mDevice, &descriptorSetLayoutCreateInfo, nullptr, &mPipelineDescriptorSetLayout);

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutList{ mPipelineDescriptorSetLayout };
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = NULL;
        pipelineLayoutCreateInfo.flags = 0;
        pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutList.size();
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayoutList.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = NULL;
        vkTools::VkErrorCheck(vkCreatePipelineLayout(mDevice, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout));
         
        VkDescriptorPoolSize descriptorPoolSize;
        descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorPoolSize.descriptorCount = 1;
        std::vector<VkDescriptorPoolSize> descriptorPoolSizeList{ descriptorPoolSize };

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.pNext = NULL;
        descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizeList.data();
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizeList.size();
        vkTools::VkErrorCheck(vkCreateDescriptorPool(mDevice, &descriptorPoolCreateInfo, nullptr, &mPipelineDescriptorPool));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.pNext = NULL;
        descriptorSetAllocateInfo.descriptorPool = mPipelineDescriptorPool;
        descriptorSetAllocateInfo.pSetLayouts = &mPipelineDescriptorSetLayout;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        vkTools::VkErrorCheck(vkAllocateDescriptorSets(mDevice, &descriptorSetAllocateInfo, &mPipelineDescriptorSet));

        std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList{
            vkTools::CreatePipelineShaderStageCreateInfo(mDevice, mVertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT, "main"),
            vkTools::CreatePipelineShaderStageCreateInfo(mDevice, mGeometryShaderModule, VK_SHADER_STAGE_GEOMETRY_BIT, "main"),
            vkTools::CreatePipelineShaderStageCreateInfo(mDevice, mPixelShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT, "main"),
        };

        vkTools::CreateGraphicsPipeline(mDevice, mExtent, pipelineShaderStageCreateInfoList, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_FRONT_FACE_COUNTER_CLOCKWISE, mRenderPass, mPipelineLayout, mPipeline);*/
    }
}

ParticleUpdateSystem::~ParticleUpdateSystem()
{
    vkFreeMemory(mDevice, mMetaDataBufferMemory, nullptr);
    vkDestroyBuffer(mDevice, mMetaDataBuffer, nullptr);

    vkDestroyShaderModule(mDevice, mComputeShaderModule, nullptr);

    /*vkDestroyPipeline(mDevice, mPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(mDevice, mPipelineDescriptorSetLayout, nullptr);
    vkFreeDescriptorSets(mDevice, mPipelineDescriptorPool, 1, &mPipelineDescriptorSet);
    vkDestroyDescriptorPool(mDevice, mPipelineDescriptorPool, nullptr);*/
}

void ParticleUpdateSystem::Update(VkCommandBuffer commandBuffer, Scene* scene, float dt)
{
    scene->mParticleBuffer->GetOutputBuffer()->Copy(commandBuffer, scene->mParticleBuffer->GetInputBuffer());

//    //mpDeviceContext->CSSetShader(mComputeShader, NULL, NULL);
//    //mpDeviceContext->CSSetShaderResources(0, 1, &scene->mParticleBuffer->GetInputBuffer()->mSRV);
//    //{
//    //    mUpdateMetaData.dt = dt;
//    //    mUpdateMetaData.particleCount = scene->mParticleCount;
//    //    DxHelp::WriteStructuredBuffer<UpdateMetaData>(mpDeviceContext, &mUpdateMetaData, 1, mUpdateMetaDataBuffer);
//    //    mpDeviceContext->CSSetShaderResources(1, 1, &mUpdateMetaDataBuffer);
//    //}
//    //mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &scene->mParticleBuffer->GetOutputBuffer()->mUAV, NULL);
//
//    //mpDeviceContext->Dispatch(scene->mParticleCount / 256 + 1,1,1);
//
//    //mpDeviceContext->CSSetShader(NULL, NULL, NULL);
//    //void* p[1] = { NULL };
//    //mpDeviceContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
//    //mpDeviceContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
//    //mpDeviceContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)p, NULL);
}
