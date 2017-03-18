#include "Scene.hpp"
#include "StorageSwapBuffer.hpp"

Scene::Scene(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int maxParticleCount)
{
    mDevice = device;
    mPhysicalDevice = physicalDevice;
    mParticleCount = 0;
    mMaxParticleCount = maxParticleCount;

    mParticleBuffer = new StorageSwapBuffer(mDevice, mPhysicalDevice, sizeof(Particle) * mMaxParticleCount, sizeof(Particle));
}

Scene::~Scene()
{
    delete mParticleBuffer;
}

void Scene::AddParticles(std::vector<Particle>& particleList)
{
    unsigned int offset = mParticleCount * sizeof(Particle);
    unsigned int particleCount = (unsigned int)particleList.size();
    unsigned int bytes = particleCount * sizeof(Particle);

    mParticleBuffer->GetInputBuffer()->Write(particleList.data(), bytes, offset);

    mParticleCount += particleCount;
}
