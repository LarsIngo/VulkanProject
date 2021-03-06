#pragma once

#ifdef NDEBUG
#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#define MsgAssert(x, y, msg) if (x != y) { MessageBox(NULL, _T(msg), _T("MsgAssert"), MB_OK); }
#else
#define MsgAssert(x, y, msg)  if (x != y) { exit(-1); }
#endif // WIN32
#else
#include <assert.h>
#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#define MsgAssert(x, y, msg) if (x != y) { MessageBox(NULL, _T(msg), _T("MsgAssert"), MB_OK); assert(x == y); }
#else
#define MsgAssert(x, y, msg) if (x != y) { assert(x == y && msg); }
#endif // WIN32
#endif // NDEBUG

#include <vulkan/vulkan.h>

#include <vector>
#include <string>

namespace vkTools 
{
    void VkErrorCheck( const VkResult& result );

    void ReadSPV( const std::string& file_path, std::vector<char>& output );

    void CreateShaderModule( const VkDevice& device, const char* shader_spv_path, VkShaderModule& shader_module );

    VkPipelineShaderStageCreateInfo CreatePipelineShaderStageCreateInfo( const VkDevice& device, const VkShaderModule& shader_module, const VkShaderStageFlagBits& stage_bit, const char* name );

    void CreateRenderPass( const VkDevice& device, const VkFormat& format, const VkImageLayout& initial_layout, const VkImageLayout& final_layout, const VkFormat& depth_format, VkRenderPass& render_pass );

    void CreateFramebuffer( const VkDevice& device, const VkExtent2D extent, const VkRenderPass& render_pass, const VkImageView& color_image_view, const VkImageView& depth_image_view, VkFramebuffer& framebuffer );

    void CreateGraphicsPipeline( const VkDevice& device,
        const VkExtent2D& extent,
        const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_list,
        const VkPrimitiveTopology& topology,
        const VkFrontFace& frontFace,
        const VkRenderPass& render_pass,
        const VkPipelineLayout& pipeline_layout,
        VkPipeline& graphics_pipeline );

    // https://gist.github.com/sheredom/523f02bbad2ae397d7ed255f3f3b5a7f
    void FindGraphicsFamily( const VkPhysicalDevice& gpu, uint32_t& family_index, uint32_t& queue_count );
    void FindTransferFamily( const VkPhysicalDevice& gpu, uint32_t& family_index, uint32_t& queue_count );
    void FindComputeFamily( const VkPhysicalDevice& gpu, uint32_t& family_index, uint32_t& queue_count );
    void PrintFamilyIndices( const VkPhysicalDevice& gpu );

    uint32_t FindPresentFamilyIndex( const VkPhysicalDevice& gpu, const VkSurfaceKHR& surface );
    uint32_t FindMemoryType( const VkPhysicalDevice& gpu, const uint32_t& type_filter, const VkMemoryPropertyFlags& memory_property_flags );
    VkFormat FindSupportedFormat( const VkPhysicalDevice& gpu, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features );

    void TransitionImageLayout( const VkCommandBuffer& command_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout );
    void CopyImage( const VkCommandBuffer& command_buffer, VkImage src_image, VkImage dst_image, std::uint32_t width, std::uint32_t height );
    void CreateImage( const VkDevice& device, const VkPhysicalDevice& gpu, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory );
    void CreateImageView( const VkDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& image_view );

    void WriteBuffer(const VkCommandBuffer& command_buffer, VkDevice device, VkDeviceMemory src_buffer_memory, void* data, std::uint32_t byte_size, std::uint32_t byte_offset);
    void CopyBuffer( const VkCommandBuffer& command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer, std::uint32_t byte_size, std::uint32_t src_byte_offset, std::uint32_t dst_byte_offset );
    void CreateBuffer( const VkDevice& device, const VkPhysicalDevice& physical_device, std::size_t total_size, VkBufferUsageFlags buffer_usage_flags, VkMemoryPropertyFlags memory_property_flags, VkBuffer& buffer, VkDeviceMemory& buffer_memory, uint32_t& min_offset_alignment );

    VkCommandBuffer BeginSingleTimeCommand( const VkDevice& device, const VkCommandPool& command_pool );
    void EndSingleTimeCommand( const VkDevice& device, const VkCommandPool& command_pool, const VkQueue& queue, const VkCommandBuffer& command_buffer );

    void CreateVkSemaphore(const VkDevice& device, VkSemaphore& semaphore);
    void CreateCommandBuffer(const VkDevice& device, const VkCommandPool& command_pool, const VkCommandBufferLevel command_buffer_level, VkCommandBuffer& command_buffer);
    void BeginCommandBuffer(const VkCommandBufferUsageFlags command_buffer_useage_flags, const VkCommandBuffer& command_buffer);
    void BeginCommandBuffer(const VkCommandBufferUsageFlags command_buffer_useage_flags, const VkCommandBufferInheritanceInfo command_buffer_inheritance_info, const VkCommandBuffer& command_buffer);
    void EndCommandBuffer( const VkCommandBuffer& command_buffer );
    void QueueSubmit(const VkQueue& queue, const std::vector<VkCommandBuffer>& command_buffer_list = {}, const std::vector<VkSemaphore>& signal_semaphore_list = {}, VkPipelineStageFlags wait_dst_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, const std::vector<VkSemaphore>& wait_semaphore_list = {} );
    void WaitQueue(const VkQueue& queue );
    void ResetCommandBuffer( VkCommandBuffer& command_buffer );
    void FreeCommandBuffer( const VkDevice& device, const VkCommandPool& command_pool, const VkCommandBuffer& command_buffer );
}
