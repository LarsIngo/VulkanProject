#define BUILD_ENABLE_VULKAN_RUNTIME_DEBUG

#include "vkTools.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <sstream>
#include <fstream>
#include <assert.h>
#include <iostream>
#include <vulkan/vulkan.h>

void vkTools::ReadSPV( const std::string& file_path, std::vector<char>& output)
{
    std::ifstream file(file_path, std::ios::ate | std::ios::binary );

    if (!file.is_open()) {
        std::string msg("Vulkan runtime error." + std::string("READFILEERROR: failed to open file : " + file_path + "."));
        MsgAssert(1, 0, msg.c_str());
    }

    size_t fileSize = (size_t)file.tellg();
    output.resize(fileSize);

    file.seekg(0);
    file.read(output.data(), fileSize);

    file.close();
}


void vkTools::CreateShaderModule( const VkDevice& device, const char* shader_spv_path, VkShaderModule& shader_module )
{
    std::vector<char> shader_spv;
    vkTools::ReadSPV( shader_spv_path, shader_spv);
    VkShaderModuleCreateInfo shader_module_create_info = {};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = shader_spv.size();
    shader_module_create_info.pCode = (uint32_t*)shader_spv.data();
    VkErrorCheck( vkCreateShaderModule( device, &shader_module_create_info, nullptr, &shader_module ) );
}


VkPipelineShaderStageCreateInfo vkTools::CreatePipelineShaderStageCreateInfo(
    const VkDevice& device, 
    const VkShaderModule& shader_module,
    const VkShaderStageFlagBits& stage_bit, 
    const char* name )
{
    VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info = {};
    pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipeline_shader_stage_create_info.stage = stage_bit;
    pipeline_shader_stage_create_info.pName = name;
    pipeline_shader_stage_create_info.module = shader_module;
    return pipeline_shader_stage_create_info;
}


void vkTools::CreateRenderPass( const VkDevice& device, const VkFormat& format, const VkImageLayout& initial_layout, const VkImageLayout& final_layout, const VkFormat& depth_format, VkRenderPass& render_pass )
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //VK_ATTACHMENT_LOAD_OP_CLEAR
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = initial_layout;
    color_attachment.finalLayout = final_layout;

    //VkAttachmentDescription depth_attachment = {};
    //depth_attachment.format = depth_format;
    //depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    //depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //VkAttachmentReference depth_attachment_ref = {};
    //depth_attachment_ref.attachment = 1;
    //depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subPass = {};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &color_attachment_ref;
    //subPass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachment_list = { color_attachment /*, depth_attachment*/ };
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachment_list.size();
    render_pass_info.pAttachments = attachment_list.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subPass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VkErrorCheck( vkCreateRenderPass( device, &render_pass_info, nullptr, &render_pass ) );
}


void vkTools::CreateFramebuffer( const VkDevice& device, const VkExtent2D extent, const VkRenderPass& render_pass, const VkImageView& color_image_view, const VkImageView& depth_image_view, VkFramebuffer& framebuffer )
{
    std::vector<VkImageView> attachment_list = { color_image_view };
    if (depth_image_view != VK_NULL_HANDLE)
        attachment_list.push_back(depth_image_view);

    VkFramebufferCreateInfo framebuffer_create_info = {};
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.renderPass = render_pass;
    framebuffer_create_info.attachmentCount = attachment_list.size();
    framebuffer_create_info.pAttachments = attachment_list.data();
    framebuffer_create_info.width = extent.width;
    framebuffer_create_info.height = extent.height;
    framebuffer_create_info.layers = 1;

    VkErrorCheck( vkCreateFramebuffer( device, &framebuffer_create_info, nullptr, &framebuffer ) );
}


void vkTools::CreateGraphicsPipeline(
    const VkDevice& device, 
    const VkExtent2D& extent,
    const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_list,
    const VkPrimitiveTopology& topology,
    const VkFrontFace& frontFace,
    const VkRenderPass& render_pass,
    const VkPipelineLayout& pipeline_layout,
    VkPipeline& graphics_pipeline )
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>( extent.width );
    viewport.height = static_cast<float>( extent.height );
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = frontFace;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_TRUE; //VK_FALSE
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>( shader_stage_list.size() );
    pipelineInfo.pStages = shader_stage_list.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = pipeline_layout;
    pipelineInfo.renderPass = render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkErrorCheck( vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline ) );
}


//uint32_t vkTools::FindFamilyIndex( const VkPhysicalDevice& gpu, VkQueueFlagBits queue_flag_bit )
//{
    //uint32_t queue_family_count = 0;
    //vkGetPhysicalDeviceQueueFamilyProperties( gpu, &queue_family_count, nullptr );
    //std::vector<VkQueueFamilyProperties> queue_family_properties_list( queue_family_count );
    //vkGetPhysicalDeviceQueueFamilyProperties( gpu, &queue_family_count, queue_family_properties_list.data() );
    //return 0;

    //for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties_list.size()); ++i)
    //{
    //    assert(queue_family_properties_list[i].timestampValidBits);
    //    uint32_t queueCount = queue_family_properties_list[i].queueCount;
    //    VkQueueFlags queueFlags = queue_family_properties_list[i].queueCount;
    //    if (queueCount > 0 && queueFlags & queue_flag_bit)
    //        return i;
    //}
    //   
    //MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Supported queue family not found.");

    //return 0;
//}

uint32_t vkTools::FindGraphicsFamilyIndex(const VkPhysicalDevice& gpu)
{
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, 0);

    VkQueueFamilyProperties* const queueFamilyProperties = (VkQueueFamilyProperties*)_alloca(
        sizeof(VkQueueFamilyProperties) * queueFamilyPropertiesCount);

    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, queueFamilyProperties);

    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!)
        const VkQueueFlags maskedFlags = (~VK_QUEUE_SPARSE_BINDING_BIT & queueFamilyProperties[i].queueFlags);

        if (VK_QUEUE_GRAPHICS_BIT & maskedFlags) {
            return i;
        }
    }

    MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Supported queue family not found.");

    return 0;
}

uint32_t vkTools::FindTransferFamilyIndex(const VkPhysicalDevice& gpu)
{
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, 0);

    VkQueueFamilyProperties* const queueFamilyProperties = (VkQueueFamilyProperties*)_alloca(
        sizeof(VkQueueFamilyProperties) * queueFamilyPropertiesCount);

    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, queueFamilyProperties);

    // first try and find a queue that has just the transfer bit set
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!)
        const VkQueueFlags maskedFlags = (~VK_QUEUE_SPARSE_BINDING_BIT & queueFamilyProperties[i].queueFlags);

        if (!((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT) & maskedFlags) &&
            (VK_QUEUE_TRANSFER_BIT & maskedFlags)) {
            return i;
        }
    }

    // otherwise we'll prefer using a compute-only queue,
    // remember that having compute on the queue implicitly enables transfer!
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!)
        const VkQueueFlags maskedFlags = (~VK_QUEUE_SPARSE_BINDING_BIT & queueFamilyProperties[i].queueFlags);

        if (!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && (VK_QUEUE_COMPUTE_BIT & maskedFlags)) {
            return i;
        }
    }

    // lastly get any queue that'll work for us (graphics, compute or transfer bit set)
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!)
        const VkQueueFlags maskedFlags = (~VK_QUEUE_SPARSE_BINDING_BIT & queueFamilyProperties[i].queueFlags);

        if ((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) & maskedFlags) {
            return i;
        }
    }

    MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Supported queue family not found.");

    return 0;
}

uint32_t vkTools::FindComputeFamilyIndex(const VkPhysicalDevice& gpu)
{
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, 0);

    VkQueueFamilyProperties* const queueFamilyProperties = (VkQueueFamilyProperties*)_alloca(
        sizeof(VkQueueFamilyProperties) * queueFamilyPropertiesCount);

    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, queueFamilyProperties);

    // first try and find a queue that has just the compute bit set
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) &
            queueFamilyProperties[i].queueFlags);

        if (!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && (VK_QUEUE_COMPUTE_BIT & maskedFlags)) {
            return i;
        }
    }

    // lastly get any queue that'll work for us
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) &
            queueFamilyProperties[i].queueFlags);

        if (VK_QUEUE_COMPUTE_BIT & maskedFlags) {
            return i;
        }
    }

    MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Supported queue family not found.");

    return 0;
}

void vkTools::PrintFamilyIndices(const VkPhysicalDevice& gpu)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_properties_list(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_family_properties_list.data());

    for (std::size_t i = 0; i < queue_family_properties_list.size(); ++i)
    {
        VkQueueFamilyProperties& queue_family_properties = queue_family_properties_list[i];
        std::cout << "Family index: " << i << std::endl;
        std::cout << "timestampValidBits: " << queue_family_properties.timestampValidBits << std::endl;
        std::cout << "queueCount: " << queue_family_properties.queueCount << std::endl;
        std::cout << "queueFlags: ";
        if (queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            std::cout << "VK_QUEUE_GRAPHICS_BIT ";
        if (queue_family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT)
            std::cout << "VK_QUEUE_COMPUTE_BIT ";
        if (queue_family_properties.queueFlags & VK_QUEUE_TRANSFER_BIT)
            std::cout << "VK_QUEUE_TRANSFER_BIT ";
        if (queue_family_properties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
            std::cout << "VK_QUEUE_SPARSE_BINDING_BIT ";
        std::cout << std::endl << std::endl;
    }
}

uint32_t vkTools::FindPresentFamilyIndex(const VkPhysicalDevice& gpu, const VkSurfaceKHR& surface)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_properties_list(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_family_properties_list.data());

    for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties_list.size()); ++i) {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &presentSupport);
        if (queue_family_properties_list[i].queueCount > 0 && presentSupport)
            return i;
    }

    MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Supported queue family not found.");

    return 0;
}

uint32_t vkTools::FindMemoryType( const VkPhysicalDevice& gpu, const uint32_t& type_filter, const VkMemoryPropertyFlags& memory_property_flags )
{
    VkPhysicalDeviceMemoryProperties gpu_memory_properties;
    vkGetPhysicalDeviceMemoryProperties( gpu, &gpu_memory_properties );

    for ( uint32_t i = 0; i < gpu_memory_properties.memoryTypeCount; ++i ) {
        if ( (type_filter & (1 << i)) && ( gpu_memory_properties.memoryTypes[i].propertyFlags & memory_property_flags ) == memory_property_flags ) {
            return i;
        }
    }
    MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Memory type not found.");

    return 0;
}


VkFormat vkTools::FindSupportedFormat( const VkPhysicalDevice& gpu, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features )
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Supported format not found.");

    return VK_FORMAT_UNDEFINED;
}


void vkTools::TransitionImageLayout(const VkCommandBuffer& command_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    if (old_layout == new_layout)
        return;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
        barrier.srcAccessMask = 0;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0; //?
    }
    else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; //?
    }
    else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; //?
    }
    else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; //?
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    }
    else {
        MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Unsupported layout transition.");
    }

    if (new_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
        barrier.dstAccessMask = 0;
    }
    else if (new_layout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.dstAccessMask = 0; //?
    }
    else if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    else if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }
    else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (new_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
        barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT; //?
    }
    else if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    }
    else {
        MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Unsupported layout transition.");
    }

    vkCmdPipelineBarrier(
        command_buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}


void vkTools::CopyImage(const VkCommandBuffer& command_buffer, VkImage src_image, VkImage dst_image, std::uint32_t width, std::uint32_t height)
{
    VkImageSubresourceLayers subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.baseArrayLayer = 0;
    subresource.mipLevel = 0;
    subresource.layerCount = 1;

    VkImageCopy region = {};
    region.srcSubresource = subresource;
    region.dstSubresource = subresource;
    region.srcOffset = { 0, 0, 0 };
    region.dstOffset = { 0, 0, 0 };
    region.extent.width = width;
    region.extent.height = height;
    region.extent.depth = 1;

    vkCmdCopyImage(
        command_buffer,
        src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region
    );
}


void vkTools::CreateImage(const VkDevice& device, const VkPhysicalDevice& gpu, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory)
{
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkErrorCheck((vkCreateImage(device, &image_info, nullptr, &image)));


    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device, image, &mem_requirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = mem_requirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(gpu, mem_requirements.memoryTypeBits, properties);

    VkErrorCheck((vkAllocateMemory(device, &allocInfo, nullptr, &image_memory)));

    vkBindImageMemory(device, image, image_memory, 0);
}


void vkTools::CreateImageView(const VkDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& image_view)
{
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspectFlags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkErrorCheck(vkCreateImageView(device, &view_info, nullptr, &image_view));
}


void vkTools::WriteBuffer( const VkCommandBuffer& command_buffer, VkDevice device, VkDeviceMemory src_buffer_memory, void* data, std::uint32_t byte_size, std::uint32_t byte_offset )
{
    void* pDeviceMemory;
    vkTools::VkErrorCheck(vkMapMemory(device, src_buffer_memory, byte_offset, byte_size, 0, &pDeviceMemory));
    std::memcpy(pDeviceMemory, data, byte_size);
    vkUnmapMemory(device, src_buffer_memory);
}


void vkTools::CopyBuffer( const VkCommandBuffer& command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer, std::uint32_t byte_size, std::uint32_t src_byte_offset, std::uint32_t dst_byte_offset )
{
    VkBufferCopy region;
    region.size = byte_size;
    region.srcOffset = src_byte_offset;
    region.dstOffset = dst_byte_offset;

    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &region);
}

void vkTools::CreateBuffer( const VkDevice& device, const VkPhysicalDevice& physical_device, std::size_t total_size, VkBufferUsageFlags buffer_usage_flags, VkMemoryPropertyFlags memory_property_flags, VkBuffer& buffer, VkDeviceMemory& buffer_memory, uint32_t& min_offset_alignment )
{
    VkPhysicalDeviceProperties physical_device_proterties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_proterties);

    if (buffer_usage_flags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        min_offset_alignment = physical_device_proterties.limits.minUniformBufferOffsetAlignment;
    else if (buffer_usage_flags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        min_offset_alignment = physical_device_proterties.limits.minStorageBufferOffsetAlignment;
    else
        MsgAssert(1, 0, "Vulkan runtime error. VKERROR: Unsupported buffer type.");

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = total_size;
    buffer_create_info.usage = buffer_usage_flags;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkTools::VkErrorCheck(vkCreateBuffer(device, &buffer_create_info, nullptr, &buffer));

    {
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info = {};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = vkTools::FindMemoryType(physical_device, memory_requirements.memoryTypeBits, memory_property_flags);

        vkTools::VkErrorCheck(vkAllocateMemory(device, &memory_allocate_info, nullptr, &buffer_memory));
    }

    vkTools::VkErrorCheck(vkBindBufferMemory(device, buffer, buffer_memory, 0));
}


VkCommandBuffer vkTools::BeginSingleTimeCommand( const VkDevice& device, const VkCommandPool& command_pool ) {
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkErrorCheck( vkAllocateCommandBuffers( device, &command_buffer_allocate_info, &command_buffer) );

    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkErrorCheck( vkBeginCommandBuffer( command_buffer, &command_buffer_begin_info ) );

    return command_buffer;
}


void vkTools::EndSingleTimeCommand( const VkDevice& device, const VkCommandPool& command_pool, const VkQueue& queue, const VkCommandBuffer& command_buffer ) {
    VkErrorCheck( vkEndCommandBuffer( command_buffer ) );

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;

    VkErrorCheck( vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE ) );
    VkErrorCheck( vkQueueWaitIdle( queue ) );

    vkFreeCommandBuffers( device, command_pool, 1, &command_buffer );
}


void vkTools::CreateCommandBuffer(const VkDevice& device, const VkCommandPool& command_pool, const VkCommandBufferLevel command_buffer_level, VkCommandBuffer& command_buffer)
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = command_buffer_level;

    VkErrorCheck(vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffer));
}


void vkTools::BeginCommandBuffer( const VkCommandBufferUsageFlags command_buffer_useage_flags, const VkCommandBuffer& command_buffer ) {
    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = command_buffer_useage_flags;

    VkErrorCheck(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));
}


void vkTools::BeginCommandBuffer(const VkCommandBufferUsageFlags command_buffer_useage_flags, const VkCommandBufferInheritanceInfo command_buffer_inheritance_info, const VkCommandBuffer& command_buffer) {
    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = command_buffer_useage_flags;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    VkErrorCheck(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));
}


void vkTools::EndCommandBuffer( const VkCommandBuffer& command_buffer ) {
    VkErrorCheck(vkEndCommandBuffer(command_buffer));
}


void vkTools::SubmitCommandBuffer(const VkQueue& queue, const std::vector<VkCommandBuffer>& command_buffer_list, const std::vector<VkSemaphore>& signal_semaphore_list, VkPipelineStageFlags wait_dst_stage_flags, const std::vector<VkSemaphore>& wait_semaphore_list ) {
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = wait_semaphore_list.size();
    submit_info.pWaitSemaphores = wait_semaphore_list.data();
    submit_info.pWaitDstStageMask = &wait_dst_stage_flags;
    submit_info.commandBufferCount = command_buffer_list.size();
    submit_info.pCommandBuffers = command_buffer_list.data();
    submit_info.signalSemaphoreCount = signal_semaphore_list.size();
    submit_info.pSignalSemaphores = signal_semaphore_list.data();

    VkErrorCheck(vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE));
}


void vkTools::WaitQueue( const VkQueue& queue ) {
    VkErrorCheck(vkQueueWaitIdle(queue));
}


void vkTools::ResetCommandBuffer( VkCommandBuffer& command_buffer ) {
    vkResetCommandBuffer(command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}


void vkTools::FreeCommandBuffer( const VkDevice& device, const VkCommandPool& command_pool, const VkCommandBuffer& command_buffer ) {
    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}


#ifdef BUILD_ENABLE_VULKAN_RUNTIME_DEBUG

void vkTools::VkErrorCheck( const VkResult& result )
{
    std::ostringstream stream;
    if ( result != VK_SUCCESS ) {
        stream << "VKERROR CHECK: " << std::endl;
        switch ( result ) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            stream << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            stream << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            stream << "VK_ERROR_INITIALIZATION_FAILED" << std::endl;
            break;
        case VK_ERROR_DEVICE_LOST:
            stream << "VK_ERROR_DEVICE_LOST" << std::endl;
            break;
        case VK_ERROR_MEMORY_MAP_FAILED:
            stream << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
            break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            stream << "VK_ERROR_LAYER_NOT_PRESENT" << std::endl;
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            stream << "VK_ERROR_EXTENSION_NOT_PRESENT" << std::endl;
            break;
        case VK_ERROR_FEATURE_NOT_PRESENT:
            stream << "VK_ERROR_FEATURE_NOT_PRESENT" << std::endl;
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            stream << "VK_ERROR_INCOMPATIBLE_DRIVER" << std::endl;
            break;
        case VK_ERROR_TOO_MANY_OBJECTS:
            stream << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            stream << "VK_ERROR_FORMAT_NOT_SUPPORTED" << std::endl;
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            stream << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;
            break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            stream << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" << std::endl;
            break;
        case VK_SUBOPTIMAL_KHR:
            stream << "VK_SUBOPTIMAL_KHR" << std::endl;
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            stream << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
            break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            stream << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" << std::endl;
            break;
        case VK_ERROR_VALIDATION_FAILED_EXT:
            stream << "VK_ERROR_VALIDATION_FAILED_EXT" << std::endl;
            break;
        case VK_ERROR_INVALID_SHADER_NV:
            stream << "VK_ERROR_INVALID_SHADER_NV" << std::endl;
            break;
        default:
            break;
        }

        std::ostringstream errMsg;
        errMsg << "Vulkan runtime error: " << stream.str();
        MsgAssert(1, 0, errMsg.str().c_str());
    }
}

#else

void vkTools::VkErrorCheck( VkResult result ) {};

#endif //BUILD_ENABLE_VULKAN_RUNTIME_DEBUG
