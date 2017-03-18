#pragma once

#include <vector>
#include "Particle.hpp"
#include <vulkan/vulkan.h>

class StorageSwapBuffer;
class ParticleSystem;

class Scene
{
    friend ParticleSystem;

    public:
        // Constructor.
        // device Vulkan device.
        // physicalDevice Vulkan physical device.
        // maxParticleCount Max number of particles in scene.
        Scene(VkDevice device, VkPhysicalDevice physicalDevice, unsigned int maxParticleCount);

        // Destructor.
        ~Scene();

        // Adds partilces to scene.
        // commandBuffer Command buffer to make device copy.
        // particleList Vector of particles to add.
        void AddParticles(VkCommandBuffer commandBuffer, std::vector<Particle>& particleList);

    private:
        unsigned int mMaxParticleCount;
        unsigned int mParticleCount;
        StorageSwapBuffer* mParticleBuffer;

        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;
};
