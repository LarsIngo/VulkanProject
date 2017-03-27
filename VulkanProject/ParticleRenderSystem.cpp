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

    // Create meta buffer.
    uint32_t minOffsetAligment;
    vkTools::CreateBuffer(mDevice, mPhysicalDevice, sizeof(MetaData),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mMetaDataBuffer, mMetaDataBufferMemory, minOffsetAligment
    );
    assert(sizeof(MetaData) % minOffsetAligment == 0);

    // Create render pipeline.
    {
        vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Render_VS.spv", mVertexShaderModule);
        vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Render_GS.spv", mGeometryShaderModule);
        vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Render_PS.spv", mPixelShaderModule);

        VkDescriptorSetLayoutBinding particleBufferSetLayoutBinding;
        particleBufferSetLayoutBinding.descriptorCount = 1;
        particleBufferSetLayoutBinding.pImmutableSamplers = nullptr;
        particleBufferSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        particleBufferSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        particleBufferSetLayoutBinding.binding = 0;
        VkDescriptorSetLayoutBinding metaBufferSetLayoutBinding;
        metaBufferSetLayoutBinding.descriptorCount = 1;
        metaBufferSetLayoutBinding.pImmutableSamplers = nullptr;
        metaBufferSetLayoutBinding.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
        metaBufferSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        metaBufferSetLayoutBinding.binding = 1;
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList{ particleBufferSetLayoutBinding, metaBufferSetLayoutBinding };

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
        descriptorPoolSize.descriptorCount = descriptorSetLayoutBindingList.size();
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

        vkTools::CreateGraphicsPipeline(mDevice, mExtent, pipelineShaderStageCreateInfoList, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_FRONT_FACE_CLOCKWISE, mRenderPass, mPipelineLayout, mPipeline);
    }
}

ParticleRenderSystem::~ParticleRenderSystem()
{
    vkFreeMemory(mDevice, mMetaDataBufferMemory, nullptr);
    vkDestroyBuffer(mDevice, mMetaDataBuffer, nullptr);

    vkDestroyShaderModule(mDevice, mVertexShaderModule, nullptr);
    vkDestroyShaderModule(mDevice, mGeometryShaderModule, nullptr);
    vkDestroyShaderModule(mDevice, mPixelShaderModule, nullptr);

    vkDestroyPipeline(mDevice, mPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(mDevice, mPipelineDescriptorSetLayout, nullptr);
    vkFreeDescriptorSets(mDevice, mPipelineDescriptorPool, 1, &mPipelineDescriptorSet);
    vkDestroyDescriptorPool(mDevice, mPipelineDescriptorPool, nullptr);
}

void ParticleRenderSystem::Render(VkCommandBuffer commandBuffer, Scene* scene, Camera* camera)
{
    assert(camera->mpFrameBuffer->mWidth == mExtent.width && camera->mpFrameBuffer->mHeight == mExtent.height);

    mMetaData.vpMatrix = glm::transpose(camera->mProjectionMatrix * camera->mViewMatrix);
    mMetaData.lensPosition = glm::vec4(camera->mPosition, 0.f);
    mMetaData.lensUpDirection = glm::vec4(camera->mUpDirection, 0.f);
    vkTools::WriteBuffer(commandBuffer, mDevice, mMetaDataBufferMemory, &mMetaData, sizeof(MetaData), 0);

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

    {   // vkUpdateDescriptorSets.
        VkDescriptorBufferInfo particleBufferInputDescriptorBufferInfo;
        VkWriteDescriptorSet particleBufferInputWriteDescriptorSet;
        particleBufferInputDescriptorBufferInfo.buffer = scene->mParticleBuffer->GetOutputBuffer()->mBuffer;
        particleBufferInputDescriptorBufferInfo.offset = 0;
        particleBufferInputDescriptorBufferInfo.range = scene->mParticleBuffer->GetOutputBuffer()->GetSize();
        particleBufferInputWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        particleBufferInputWriteDescriptorSet.pNext = NULL;
        particleBufferInputWriteDescriptorSet.dstSet = mPipelineDescriptorSet;
        particleBufferInputWriteDescriptorSet.dstArrayElement = 0;
        particleBufferInputWriteDescriptorSet.descriptorCount = 1;
        particleBufferInputWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        particleBufferInputWriteDescriptorSet.pImageInfo = NULL;
        particleBufferInputWriteDescriptorSet.dstBinding = 0;
        particleBufferInputWriteDescriptorSet.pBufferInfo = &particleBufferInputDescriptorBufferInfo;

        VkDescriptorBufferInfo metaBufferInputDescriptorBufferInfo;
        VkWriteDescriptorSet metaBufferInputWriteDescriptorSet;
        metaBufferInputDescriptorBufferInfo.buffer = mMetaDataBuffer;
        metaBufferInputDescriptorBufferInfo.offset = 0;
        metaBufferInputDescriptorBufferInfo.range = sizeof(MetaData);
        metaBufferInputWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        metaBufferInputWriteDescriptorSet.pNext = NULL;
        metaBufferInputWriteDescriptorSet.dstSet = mPipelineDescriptorSet;
        metaBufferInputWriteDescriptorSet.dstArrayElement = 0;
        metaBufferInputWriteDescriptorSet.descriptorCount = 1;
        metaBufferInputWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        metaBufferInputWriteDescriptorSet.pImageInfo = NULL;
        metaBufferInputWriteDescriptorSet.dstBinding = 1;
        metaBufferInputWriteDescriptorSet.pBufferInfo = &metaBufferInputDescriptorBufferInfo;

        std::vector<VkWriteDescriptorSet> writeDescriptorSetList{ particleBufferInputWriteDescriptorSet, metaBufferInputWriteDescriptorSet };
        vkUpdateDescriptorSets(mDevice, writeDescriptorSetList.size(), writeDescriptorSetList.data(), 0, NULL);
    }

    camera->mpFrameBuffer->TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mPipelineDescriptorSet, 0, NULL);
    vkCmdDraw(commandBuffer, scene->mParticleCount, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    scene->mParticleBuffer->Swap();
}
