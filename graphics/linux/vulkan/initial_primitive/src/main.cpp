#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <fstream>
#include <limits>
#include <optional>
#include <set>
#include <vector>

int main(int argc, char** argv) 
{
    // GLFW initialization
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    // Application information
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Initial Vulkan Primitive";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "None";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    // Create information
    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    uint32_t available_layers_count;
    vkEnumerateInstanceLayerProperties(&available_layers_count, nullptr);

    std::vector<VkLayerProperties> available_layers(available_layers_count);
    vkEnumerateInstanceLayerProperties(&available_layers_count, available_layers.data());

    std::set<std::string> required_layers(validation_layers.begin(), validation_layers.end());

    for (const auto& layer : available_layers)
    {
        required_layers.erase(layer.layerName);
    }

    if (!required_layers.empty())
    {
        throw std::runtime_error("Required layers are unavailable");
    }

    instance_create_info.enabledExtensionCount = glfw_extension_count;
    instance_create_info.ppEnabledExtensionNames = glfw_extensions;
    instance_create_info.enabledLayerCount = 1;
    instance_create_info.ppEnabledLayerNames = validation_layers.data();

    // Instance creation
    VkInstance instance;

    if (vkCreateInstance(&instance_create_info, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    // Render surface creation
    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create surface rendering target");
    }

    // Physical device enumeration
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0)
    {
        throw std::runtime_error("No devices with Vulkan support found");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    // Physical device property identification
    VkPhysicalDeviceFeatures device_features;
    bool suitable_device_found = false;

    for (const auto& device: devices)
    {
        vkGetPhysicalDeviceFeatures(device, &device_features);

        if (device_features.geometryShader)
        {
            suitable_device_found = true;
            physical_device = device;
            break;
        }
    }

    if (!suitable_device_found)
    {
        throw std::runtime_error("No suitable GPU with geometry shader support");
    }

    // Swapchain extension validation
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    const std::vector<const char*> required_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());
    std::set<std::string> required_extensions(required_device_extensions.begin(), required_device_extensions.end());

    for (const auto& extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    if (!required_extensions.empty())
    {
        throw std::runtime_error("One or more required device extensions are unavailable");
    }

    // Swapchain formatting and presentation validation
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

    if (format_count == 0)
    {
        throw std::runtime_error("No supported output formats found for swapchain");
    }

    uint32_t presentation_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentation_count, nullptr);

    if (presentation_count == 0)
    {
        throw std::runtime_error("No supported presentation formats found for swapchain");
    }

    std::vector<VkSurfaceFormatKHR> supported_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, supported_formats.data());

    VkPresentModeKHR selected_mode = VK_PRESENT_MODE_FIFO_KHR;
    std::vector<VkPresentModeKHR> presentation_formats(presentation_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentation_count, presentation_formats.data());

    for (const auto& mode : presentation_formats)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            selected_mode = mode;
            break;
        }
    }

    // Extent selection
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    VkExtent2D selected_extent;

    if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        selected_extent = surface_capabilities.currentExtent;
    } 
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        selected_extent = VkExtent2D{
            std::clamp(static_cast<uint32_t>(width), surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width),
            std::clamp(static_cast<uint32_t>(height), surface_capabilities.maxImageExtent.width, surface_capabilities.maxImageExtent.height)
        };
    }

    // Queue family enumeration
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
    
    uint32_t index = 0;
    std::optional<uint32_t> graphics_queue_index;
    std::optional<uint32_t> present_queue_index;

    for (const auto& queue_family : queue_families)
    {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, index, surface, &present_support);

        if (present_support)
        {
            present_queue_index = index;
        }

        if (queue_family.queueFlags && VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_queue_index = index;
        }

        ++index;
    }

    if (!graphics_queue_index.has_value())
    {
        throw std::runtime_error("No queue family with graphics command support found");
    }

    if (!present_queue_index.has_value())
    {
        throw std::runtime_error("No queue family with present support found");
    }

    // Logical device creation
    VkDevice logical_device;
    std::vector<VkDeviceQueueCreateInfo> queue_creation_infos;
    std::set<uint32_t> unique_queue_indicies = {graphics_queue_index.value(), present_queue_index.value()};

    for (const auto& queue_index : unique_queue_indicies)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_index;
        queue_create_info.queueCount = 1;

        float queue_priority = 1.0f;
        queue_create_info.pQueuePriorities = &queue_priority;

        queue_creation_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures logical_device_features{};
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = queue_creation_infos.data();
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_creation_infos.size());
    device_create_info.pEnabledFeatures = &logical_device_features;
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size());
    device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
    device_create_info.enabledLayerCount = 0;

    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device");
    }

    // Queue handle retrieval
    VkQueue graphics_queue;
    VkQueue present_queue;
    vkGetDeviceQueue(logical_device, graphics_queue_index.value(), 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, present_queue_index.value(), 0, &present_queue);

    // Swapchain creation
    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = supported_formats[0].format;
    swapchain_create_info.imageColorSpace = supported_formats[0].colorSpace;
    swapchain_create_info.imageExtent = selected_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = selected_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    const uint32_t queue_family_indices[] = {graphics_queue_index.value(), present_queue_index.value()};

    if (graphics_queue_index.value() == present_queue_index.value())
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }
    else
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices; 
    }

    VkSwapchainKHR swapchain;

    if (vkCreateSwapchainKHR(logical_device, &swapchain_create_info, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create swapchain");
    }

    // Image retrieval
    uint32_t image_count;
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, nullptr);
    std::vector<VkImage> swapchain_images(image_count);
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, swapchain_images.data());
    std::vector<VkImageView> image_views(swapchain_images.size());

    for (size_t i = 0; i < swapchain_images.size(); ++i)
    {
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = swapchain_images[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = supported_formats[0].format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(logical_device, &image_view_create_info, nullptr, &image_views[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create image view");
        }
    }

    // Shader loading
    std::ifstream vertex_shader_file("vert.spv", std::ios::ate | std::ios::binary);

    if (!vertex_shader_file.is_open())
    {
        throw std::runtime_error("Unable to read vertex shader SPIR-V");
    }

    size_t vertex_file_size = (size_t) vertex_shader_file.tellg();
    std::vector<char> vertex_shader_bytecode(vertex_file_size);

    vertex_shader_file.seekg(0);
    vertex_shader_file.read(vertex_shader_bytecode.data(), vertex_file_size);
    vertex_shader_file.close();

    std::ifstream fragment_shader_file("frag.spv", std::ios::ate | std::ios::binary);

    if (!fragment_shader_file.is_open())
    {
        throw std::runtime_error("Unable to read fragment shader SPIR-V");
    }

    size_t fragment_file_size = (size_t) fragment_shader_file.tellg();
    std::vector<char> fragment_shader_bytecode(fragment_file_size);

    fragment_shader_file.seekg(0);
    fragment_shader_file.read(fragment_shader_bytecode.data(), fragment_file_size);
    fragment_shader_file.close();

    // Shader module creation
    VkShaderModuleCreateInfo vertex_shader_module_create_info{};
    vertex_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertex_shader_module_create_info.codeSize = vertex_shader_bytecode.size();
    vertex_shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(vertex_shader_bytecode.data());
    VkShaderModule vertex_shader_module;

    if (vkCreateShaderModule(logical_device, &vertex_shader_module_create_info, nullptr, &vertex_shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create vertex shader module");
    }

    VkShaderModuleCreateInfo fragment_shader_module_create_info{};
    fragment_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragment_shader_module_create_info.codeSize = fragment_shader_bytecode.size();
    fragment_shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(fragment_shader_bytecode.data());
    VkShaderModule fragment_shader_module;

    if (vkCreateShaderModule(logical_device, &fragment_shader_module_create_info, nullptr, &fragment_shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create fragment shader module");
    }

    // Shader stage creation
    VkPipelineShaderStageCreateInfo vertex_shader_stage_creation_info{};
    vertex_shader_stage_creation_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_creation_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_creation_info.module = vertex_shader_module;
    vertex_shader_stage_creation_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_creation_info{};
    fragment_shader_stage_creation_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_creation_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_creation_info.module = fragment_shader_module;
    fragment_shader_stage_creation_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_creation_info, fragment_shader_stage_creation_info};

    // Dynamic state configuration
    std::vector<VkDynamicState> dynamic_states{};

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
    dynamic_state_create_info.pDynamicStates = dynamic_states.data();

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
    vertex_input_state_create_info.pVertexBindingDescriptions = nullptr;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_state_create_info.pVertexAttributeDescriptions = nullptr;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    // Viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)selected_extent.width;
    viewport.height = (float)selected_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = selected_extent;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.lineWidth = 1.0f;
    rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasClamp = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_create_info.minSampleShading = 1.0f;
    multisample_state_create_info.pSampleMask = nullptr;
    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_create_info.alphaToOneEnable = VK_FALSE;

    // Blending
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment;
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;

    // Color buffer attachment
    VkAttachmentDescription color_attachment{};
    color_attachment.format = supported_formats[0].format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Subpass attachment
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachment_ref;

    // Subpass dependencies
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Render Pass
    VkRenderPass render_pass;
    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &dependency;

    if (vkCreateRenderPass(logical_device, &render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create render pass");
    }

    // Pipeline layout
    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create pipeline layout");
    }

    // Graphics pipeline
    VkPipeline pipeline;
    VkGraphicsPipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    pipeline_create_info.pDepthStencilState = nullptr;
    pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    pipeline_create_info.layout = pipeline_layout;
    pipeline_create_info.renderPass = render_pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create graphics pipeline");
    }

    // Framebuffers
    std::vector<VkFramebuffer> swapchain_framebuffers(image_views.size());

    for (size_t i = 0; i < image_views.size(); ++i)
    {
        VkImageView attachments[] = {
            image_views[i]
        };

        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = attachments;
        framebuffer_create_info.width = selected_extent.width;
        framebuffer_create_info.height = selected_extent.height;
        framebuffer_create_info.layers = 1;

        if (vkCreateFramebuffer(logical_device, &framebuffer_create_info, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create framebuffer");
        }
    }

    // Command pool
    VkCommandPool command_pool;
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = graphics_queue_index.value();

    if (vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr, &command_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create command pool");
    }

    // Command buffer
    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocation_info{};
    command_buffer_allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocation_info.commandPool = command_pool;
    command_buffer_allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocation_info.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(logical_device, &command_buffer_allocation_info, &command_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to allocate command buffer");
    }

    // Synchronization
    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &image_available_semaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create available image semaphore");
    }

    if (vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &render_finished_semaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create render finished semaphore");
    }

    if (vkCreateFence(logical_device, &fence_create_info, nullptr, &in_flight_fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create in-flight fence");
    }

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Acquire next image when ready
        vkWaitForFences(logical_device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(logical_device, 1, &in_flight_fence);

        uint32_t image_index;
        vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX, image_available_semaphore, VK_NULL_HANDLE, &image_index);
        vkResetCommandBuffer(command_buffer, 0);

        // Command buffer recording
        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = 0;
        command_buffer_begin_info.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to begin recording command buffer");
        }

        // Render pass start
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = render_pass;
        render_pass_begin_info.framebuffer = swapchain_framebuffers[image_index];
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = selected_extent;

        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(command_buffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(command_buffer);

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to record command buffer");
        }

        // Command submission
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {image_available_semaphore};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount =1 ;
        submit_info.pCommandBuffers = &command_buffer;

        VkSemaphore signal_semaphores[] = {render_finished_semaphore};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;

        if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to submit draw call to command buffer");
        }

        // Presentation
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swapchains[] = {swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchains;
        present_info.pImageIndices = &image_index;
        present_info.pResults = nullptr;
        vkQueuePresentKHR(present_queue, &present_info);
    }

    // Cleanup
    vkDeviceWaitIdle(logical_device);
    vkDestroyFence(logical_device, in_flight_fence, nullptr);
    vkDestroySemaphore(logical_device, render_finished_semaphore, nullptr);
    vkDestroySemaphore(logical_device, image_available_semaphore, nullptr);
    vkDestroyCommandPool(logical_device, command_pool, nullptr);

    for (auto framebuffer : swapchain_framebuffers)
    {
        vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
    }

    vkDestroyPipeline(logical_device, pipeline, nullptr);
    vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
    vkDestroyRenderPass(logical_device, render_pass, nullptr);
    vkDestroyShaderModule(logical_device, fragment_shader_module, nullptr);
    vkDestroyShaderModule(logical_device, vertex_shader_module, nullptr);

    for (auto image_view : image_views)
    {
        vkDestroyImageView(logical_device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(logical_device, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}