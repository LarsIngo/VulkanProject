#include "ParticleRenderSystem.hpp"
#include "VkRenderer.hpp"
#include "Scene.hpp"
#include "FrameBuffer.hpp"
#include "StorageSwapBuffer.hpp"
#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "vkTools.hpp"

ParticleRenderSystem::ParticleRenderSystem(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int width, unsigned int height, VkFormat format, VkRenderPass renderPass)
{
    mDevice = device;
    mPhysicalDevice = physicalDevice;
    mExtent.width = width;
    mExtent.height = height;
    mFormat = format;
    mRenderPass = renderPass;

    // Create meta buffers.
    uint32_t minOffsetAligment;
    vkTools::CreateBuffer(mDevice, mPhysicalDevice, sizeof(UpdateMetaData),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mUpdateMetaDataBuffer, mUpdateMetaDataBufferMemory, minOffsetAligment
        );
    assert(sizeof(UpdateMetaData) % minOffsetAligment == 0);

    vkTools::CreateBuffer(mDevice, mPhysicalDevice, sizeof(RenderMetaData),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mRenderMetaDataBuffer, mRenderMetaDataBufferMemory, minOffsetAligment
    );
    int t = (sizeof(RenderMetaData));
    assert(sizeof(RenderMetaData) % minOffsetAligment == 0);

    // Create render pipeline.
    {
        vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Render_VS.spv", mVertexShaderModule);
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

        vkTools::CreateGraphicsPipeline(mDevice, mExtent, pipelineShaderStageCreateInfoList, mRenderPass, mPipelineLayout, mPipeline);
    }

    //    {   // Create blend state.
    //        D3D11_BLEND_DESC blendDesc;
    //        ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    //        blendDesc.AlphaToCoverageEnable = false;
    //        blendDesc.IndependentBlendEnable = true;

    //        blendDesc.RenderTarget[0].BlendEnable = true;
    //        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    //        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    //        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    //        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    //        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    //        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    //        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    //        DxAssert(mpDevice->CreateBlendState(&blendDesc, &mBlendState), S_OK);
    //    }
        
    VkDescriptorBufferInfo mVertexShaderInputDescriptorBufferInfo;
    VkWriteDescriptorSet mVertexShaderInputWriteDescriptorSet;
    mVertexShaderInputDescriptorBufferInfo.buffer = mRenderMetaDataBuffer;
    mVertexShaderInputDescriptorBufferInfo.offset = 0;
    mVertexShaderInputDescriptorBufferInfo.range = VK_WHOLE_SIZE;

    mVertexShaderInputWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mVertexShaderInputWriteDescriptorSet.pNext = NULL;
    mVertexShaderInputWriteDescriptorSet.dstSet = mPipelineDescriptorSet;
    mVertexShaderInputWriteDescriptorSet.dstArrayElement = 0;
    mVertexShaderInputWriteDescriptorSet.descriptorCount = 1;
    mVertexShaderInputWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    mVertexShaderInputWriteDescriptorSet.pImageInfo = NULL;
    mVertexShaderInputWriteDescriptorSet.dstBinding = 0;
    mVertexShaderInputWriteDescriptorSet.pBufferInfo = &mVertexShaderInputDescriptorBufferInfo;
    vkUpdateDescriptorSets(mDevice, 1, &mVertexShaderInputWriteDescriptorSet, 0, NULL);

    // Create update pipeline.
    //vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Update_CS.spv", mComputeShaderModule);
}

ParticleRenderSystem::~ParticleRenderSystem()
{
    vkFreeMemory(mDevice, mUpdateMetaDataBufferMemory, nullptr);
    vkFreeMemory(mDevice, mRenderMetaDataBufferMemory, nullptr);
    vkDestroyBuffer(mDevice, mUpdateMetaDataBuffer, nullptr);
    vkDestroyBuffer(mDevice, mRenderMetaDataBuffer, nullptr);

    //vkDestroyShaderModule(mDevice, mComputeShaderModule, nullptr);

    vkDestroyShaderModule(mDevice, mVertexShaderModule, nullptr);
    vkDestroyShaderModule(mDevice, mGeometryShaderModule, nullptr);
    vkDestroyShaderModule(mDevice, mPixelShaderModule, nullptr);

    vkDestroyPipeline(mDevice, mPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(mDevice, mPipelineDescriptorSetLayout, nullptr);
    vkFreeDescriptorSets(mDevice, mPipelineDescriptorPool, 1, &mPipelineDescriptorSet);
    vkDestroyDescriptorPool(mDevice, mPipelineDescriptorPool, nullptr);

    //mBlendState->Release();
}

//void ParticleSystem::Update(VkCommandBuffer commandBuffer, Scene* scene, float dt)
//{
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
//}

void ParticleRenderSystem::Render(VkCommandBuffer commandBuffer, Scene* scene, Camera* camera)
{
    assert(camera->mpFrameBuffer->mWidth == mExtent.width && camera->mpFrameBuffer->mHeight == mExtent.height);

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = NULL;
    renderPassBeginInfo.renderPass = mRenderPass;
    renderPassBeginInfo.framebuffer = camera->mpFrameBuffer->mFrameBuffer;
    renderPassBeginInfo.renderArea.extent.width = camera->mpFrameBuffer->mWidth;
    renderPassBeginInfo.renderArea.extent.height = camera->mpFrameBuffer->mHeight;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.clearValueCount = 0;
    renderPassBeginInfo.pClearValues = NULL;
    
    camera->mpFrameBuffer->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mPipelineDescriptorSet, 0, NULL);
    vkCmdDraw(commandBuffer, scene->mParticleCount, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

   /* mpDeviceContext->VSSetShader(mVertexShader, NULL, NULL);
    mpDeviceContext->GSSetShader(mGeometryShader, NULL, NULL);
    mpDeviceContext->PSSetShader(mPixelShader, NULL, NULL);
    mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    float blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
    UINT sampleMask = 0xffffffff;
    mpDeviceContext->OMSetBlendState(mBlendState, blendFactor, sampleMask);
    mpDeviceContext->OMSetRenderTargets(1, &camera->mpFrameBuffer->mColRTV, NULL);
    mpDeviceContext->VSSetShaderResources(0, 1, &scene->mParticleBuffer->GetOutputBuffer()->mSRV);
    {
        mRenderMetaData.vpMatrix = glm::transpose(camera->mProjectionMatrix * camera->mViewMatrix);
        mRenderMetaData.lensPosition = camera->mPosition;
        mRenderMetaData.lensUpDirection = camera->mUpDirection;
        DxHelp::WriteStructuredBuffer<RenderMetaData>(mpDeviceContext, &mRenderMetaData, 1, mRenderMetaDataBuffer);
        mpDeviceContext->GSSetShaderResources(0, 1, &mRenderMetaDataBuffer);
    }

    mpDeviceContext->Draw(scene->mParticleCount, 0);

    mpDeviceContext->VSSetShader(NULL, NULL, NULL);
    mpDeviceContext->GSSetShader(NULL, NULL, NULL);
    mpDeviceContext->PSSetShader(NULL, NULL, NULL);
    mpDeviceContext->OMSetBlendState(NULL, blendFactor, sampleMask);
    void* p[1] = { NULL };
    mpDeviceContext->OMSetRenderTargets(1, (ID3D11RenderTargetView**)p, NULL);
    mpDeviceContext->VSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->GSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);*/

    scene->mParticleBuffer->Swap();
}
