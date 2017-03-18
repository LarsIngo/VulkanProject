#include "ParticleSystem.hpp"
#include "VkRenderer.hpp"
#include "Scene.hpp"
#include "FrameBuffer.hpp"
#include "StorageSwapBuffer.hpp"
#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "vkTools.hpp"

ParticleSystem::ParticleSystem(VkDevice device, VkPhysicalDevice physicalDevice)
{
    mDevice = device;
    mPhysicalDevice = physicalDevice;

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

    //// Create render pipeline.
    //{
    //    DxHelp::CreateVS(mpDevice, "resources/shaders/Particles_Render_VS.hlsl", "main", &mVertexShader);
    //    DxHelp::CreateGS(mpDevice, "resources/shaders/Particles_Render_GS.hlsl", "main", &mGeometryShader);
    //    DxHelp::CreatePS(mpDevice, "resources/shaders/Particles_Render_PS.hlsl", "main", &mPixelShader);

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
    //}

    //// Create update pipeline.
    //DxHelp::CreateCS(mpDevice, "resources/shaders/Particles_Update_CS.hlsl", "main", &mComputeShader);
}

ParticleSystem::~ParticleSystem()
{
    vkFreeMemory(mDevice, mUpdateMetaDataBufferMemory, nullptr);
    vkFreeMemory(mDevice, mRenderMetaDataBufferMemory, nullptr);
    vkDestroyBuffer(mDevice, mUpdateMetaDataBuffer, nullptr);
    vkDestroyBuffer(mDevice, mRenderMetaDataBuffer, nullptr);

    //mComputeShader->Release();

    //mVertexShader->Release();
    //mGeometryShader->Release();
    //mPixelShader->Release();
    //mBlendState->Release();
}

void ParticleSystem::Update(VkCommandBuffer commandBuffer, Scene* scene, float dt)
{
    //mpDeviceContext->CSSetShader(mComputeShader, NULL, NULL);
    //mpDeviceContext->CSSetShaderResources(0, 1, &scene->mParticleBuffer->GetInputBuffer()->mSRV);
    //{
    //    mUpdateMetaData.dt = dt;
    //    mUpdateMetaData.particleCount = scene->mParticleCount;
    //    DxHelp::WriteStructuredBuffer<UpdateMetaData>(mpDeviceContext, &mUpdateMetaData, 1, mUpdateMetaDataBuffer);
    //    mpDeviceContext->CSSetShaderResources(1, 1, &mUpdateMetaDataBuffer);
    //}
    //mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &scene->mParticleBuffer->GetOutputBuffer()->mUAV, NULL);

    //mpDeviceContext->Dispatch(scene->mParticleCount / 256 + 1,1,1);

    //mpDeviceContext->CSSetShader(NULL, NULL, NULL);
    //void* p[1] = { NULL };
    //mpDeviceContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    //mpDeviceContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    //mpDeviceContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)p, NULL);
}

void ParticleSystem::Render(VkCommandBuffer commandBuffer, Scene* scene, Camera* camera)
{
    VkShaderModule mVertexShaderModule;
    vkTools::CreateShaderModule(mDevice, "resources/shaders/Particles_Render_VS.spv", mVertexShaderModule);




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
