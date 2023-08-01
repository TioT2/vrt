#include "vrt.h"
#include "vrt.h"

namespace vrt::render::core
{
  VOID kernel::InitializeSwapchain( VOID )
  {
    // Get support details
    struct
    {
      VkSurfaceCapabilitiesKHR Capabilities;
      std::vector<VkSurfaceFormatKHR> Formats;
      std::vector<VkPresentModeKHR> PresentModes;
    } SwapchainSupportDetails;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SwapchainSupportDetails.Capabilities);

    UINT32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &formatCount, nullptr);
    SwapchainSupportDetails.Formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &formatCount, SwapchainSupportDetails.Formats.data());

    UINT32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &presentModeCount, nullptr);
    SwapchainSupportDetails.PresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &presentModeCount, SwapchainSupportDetails.PresentModes.data());

    // Choose surface format and present mode
    VkSurfaceFormatKHR SurfaceFormat = SwapchainSupportDetails.Formats[0];
    for (const VkSurfaceFormatKHR format : SwapchainSupportDetails.Formats)
      if (format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB)
      {
        SurfaceFormat = format;
        break;
      }

    // choose present mode
    VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (VkPresentModeKHR mode : SwapchainSupportDetails.PresentModes)
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
      {
        PresentMode = mode;
        break;
      }

    // get swapchain extent
    if (SwapchainSupportDetails.Capabilities.currentExtent.width != UINT32_MAX)
      SwapchainImageExtent = SwapchainSupportDetails.Capabilities.currentExtent;
    else
    {
      struct
      {
        INT width, height;
      } windowExtent;

      SDL_GetWindowSize(Window, &windowExtent.width, &windowExtent.height);

      SwapchainImageExtent.width = std::clamp<UINT32>(windowExtent.width, SwapchainSupportDetails.Capabilities.minImageExtent.width, SwapchainSupportDetails.Capabilities.maxImageExtent.width);
      SwapchainImageExtent.height = std::clamp<UINT32>(windowExtent.height, SwapchainSupportDetails.Capabilities.minImageExtent.height, SwapchainSupportDetails.Capabilities.maxImageExtent.height);
    }

    UINT32 imageCount =
      SwapchainSupportDetails.Capabilities.maxImageCount == 0 ?
        SwapchainSupportDetails.Capabilities.minImageCount + 1 :
        SwapchainSupportDetails.Capabilities.maxImageCount;

    // create swapchain
    VkSwapchainCreateInfoKHR CreateInfo
    {
      /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      /* const void*                   */ .pNext = nullptr,
      /* VkSwapchainCreateFlagsKHR     */ .flags = 0,
      /* VkSurfaceKHR                  */ .surface = Surface,
      /* uint32_t                      */ .minImageCount = imageCount,
      /* VkFormat                      */ .imageFormat = SurfaceFormat.format,
      /* VkColorSpaceKHR               */ .imageColorSpace = SurfaceFormat.colorSpace,
      /* VkExtent2D                    */ .imageExtent = SwapchainImageExtent,
      /* uint32_t                      */ .imageArrayLayers = 1,
      /* VkImageUsageFlags             */ .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      /* VkSharingMode                 */ // .imageSharingMode = 0,
      /* uint32_t                      */ // .queueFamilyIndexCount = 0,
      /* const uint32_t*               */ // .pQueueFamilyIndices = 0,
      /* VkSurfaceTransformFlagBitsKHR */ .preTransform = SwapchainSupportDetails.Capabilities.currentTransform,
      /* VkCompositeAlphaFlagBitsKHR   */ .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      /* VkPresentModeKHR              */ .presentMode = PresentMode,
      /* VkBool32                      */ .clipped = VK_TRUE,
      /* VkSwapchainKHR                */ .oldSwapchain = Swapchain,
    };

    utils::AssertResult(vkCreateSwapchainKHR(Device, &CreateInfo, nullptr, &Swapchain), "Error creating swapchain.");

    UINT32 SwapchainImageCount = 0;
    vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr);
    SwapchainImages.resize(SwapchainImageCount);
    vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data());

    SwapchainImageFormat = SurfaceFormat.format;

    // create swapchain image views
    SwapchainImageViews.resize(SwapchainImages.size());
    for (SIZE_T i = 0; i < SwapchainImageViews.size(); i++)
      SwapchainImageViews[i] = CreateImageView(SwapchainImages[i], SwapchainImageFormat);
  } /* InitializeSwapchain */

  
  /* Framebuffers initialization function */
  VOID kernel::InitializeFramebuffers( VOID )
  {
    Framebuffers.resize(SwapchainImageViews.size());
    for (SIZE_T i = 0; i < Framebuffers.size(); i++)
    {
      VkFramebufferCreateInfo CreateInfo
      {
        /* VkStructureType          */ .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        /* const void*              */ .pNext = nullptr,
        /* VkFramebufferCreateFlags */ .flags = 0,
        /* VkRenderPass             */ .renderPass = PresentRenderPass,
        /* uint32_t                 */ .attachmentCount = 1,
        /* const VkImageView*       */ .pAttachments = &SwapchainImageViews[i],
        /* uint32_t                 */ .width = SwapchainImageExtent.width,
        /* uint32_t                 */ .height = SwapchainImageExtent.height,
        /* uint32_t                 */ .layers = 1,
      };

      utils::AssertResult(vkCreateFramebuffer(Device, &CreateInfo, nullptr, &Framebuffers[i]));
    }
  } /* InitializeFramebuffers */

  /* Presentation renderpass initialization function */
  VOID kernel::InitializePresentRenderPass( VOID )
  {
    VkAttachmentDescription ColorAttachmentDescription
    {
      /* VkAttachmentDescriptionFlags */ .flags = 0,
      /* VkFormat                     */ .format = VK_FORMAT_B8G8R8A8_SRGB,
      /* VkSampleCountFlagBits        */ .samples = VK_SAMPLE_COUNT_1_BIT,
      /* VkAttachmentLoadOp           */ .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      /* VkAttachmentStoreOp          */ .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      /* VkAttachmentLoadOp           */ .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      /* VkAttachmentStoreOp          */ .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      /* VkImageLayout                */ .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      /* VkImageLayout                */ .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference ColorAttachmentReference
    {
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription SubpassDescription
    {
      /* VkSubpassDescriptionFlags    */ .flags = 0,
      /* VkPipelineBindPoint          */ .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      /* uint32_t                     */ .inputAttachmentCount = 0,
      /* const VkAttachmentReference* */ .pInputAttachments = nullptr,
      /* uint32_t                     */ .colorAttachmentCount = 1,
      /* const VkAttachmentReference* */ .pColorAttachments = &ColorAttachmentReference,
      /* const VkAttachmentReference* */ .pResolveAttachments = nullptr,
      /* const VkAttachmentReference* */ .pDepthStencilAttachment = nullptr,
      /* uint32_t                     */ .preserveAttachmentCount = 0,
      /* const uint32_t*              */ .pPreserveAttachments = nullptr,
    };

    VkSubpassDependency SubpassDependency
    {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = 0,
    };

    VkRenderPassCreateInfo CreateInfo
    {
      /* VkStructureType                */ .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      /* const void*                    */ .pNext = nullptr,
      /* VkRenderPassCreateFlags        */ .flags = 0,
      /* uint32_t                       */ .attachmentCount = 1,
      /* const VkAttachmentDescription* */ .pAttachments = &ColorAttachmentDescription,
      /* uint32_t                       */ .subpassCount = 1,
      /* const VkSubpassDescription*    */ .pSubpasses = &SubpassDescription,
      /* uint32_t                       */ .dependencyCount = 1,
      /* const VkSubpassDependency*     */ .pDependencies = &SubpassDependency,
    };

    utils::AssertResult(vkCreateRenderPass(Device, &CreateInfo, nullptr, &PresentRenderPass));
  } /* InitializePresentRenderPass */


  /* Initialize image for RT rendering to */
  VOID kernel::InitializeTarget( VOID )
  {
    TargetImage = CreateImage(SwapchainImageExtent.width, SwapchainImageExtent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    TransferImageLayout(TargetImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL); // reset image layout

    VkSamplerCreateInfo SamplerCreateInfo
    {
      /* VkStructureType      */ .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      /* const void*          */ .pNext = nullptr,
      /* VkSamplerCreateFlags */ .flags = 0,
      /* VkFilter             */ .magFilter = VK_FILTER_NEAREST,
      /* VkFilter             */ .minFilter = VK_FILTER_NEAREST,
      /* VkSamplerMipmapMode  */ .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
      /* VkSamplerAddressMode */ .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      /* VkSamplerAddressMode */ .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      /* VkSamplerAddressMode */ .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      /* float                */ .mipLodBias = 0,
      /* VkBool32             */ .anisotropyEnable = VK_FALSE,
      /* float                */ .maxAnisotropy = 0,
      /* VkBool32             */ .compareEnable = VK_FALSE,
      /* VkCompareOp          */ .compareOp = VK_COMPARE_OP_ALWAYS,
      /* float                */ .minLod = 0,
      /* float                */ .maxLod = 0,
      /* VkBorderColor        */ .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
      /* VkBool32             */ .unnormalizedCoordinates = VK_FALSE,
    };

    utils::AssertResult(vkCreateSampler(Device, &SamplerCreateInfo, nullptr, &TargetImageSampler));
  } /* InitializeTarget */


  /* Presentation pipeline initialization function */
  VOID kernel::InitializePresentPipeline( VOID )
  {
    VkDescriptorSetLayoutBinding DescriptorSetBinding
    {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_ALL,
      .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo
    {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = 1,
      .pBindings = &DescriptorSetBinding,
    };

    utils::AssertResult(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, nullptr, &PresentDescriptorSetLayout));

    VkDescriptorPoolSize DescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 };

    VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo
    {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &DescriptorPoolSize,
    };

    utils::AssertResult(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, nullptr, &PresentDescriptorPool));

    VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo
    {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = PresentDescriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &PresentDescriptorSetLayout
    };

    utils::AssertResult(vkAllocateDescriptorSets(Device, &DescriptorSetAllocateInfo, &PresentDescriptorSet));

    VkDescriptorImageInfo DescriptorImageInfo
    {
      .sampler = TargetImageSampler,
      .imageView = TargetImage.ImageView,
      .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    VkWriteDescriptorSet DescriptorSetWrite
    {
      /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      /* const void*                   */ .pNext = nullptr,
      /* VkDescriptorSet               */ .dstSet = PresentDescriptorSet,
      /* uint32_t                      */ .dstBinding = 0,
      /* uint32_t                      */ .dstArrayElement = 0,
      /* uint32_t                      */ .descriptorCount = 1,
      /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      /* const VkDescriptorImageInfo*  */ .pImageInfo = &DescriptorImageInfo,
      /* const VkDescriptorBufferInfo* */ .pBufferInfo = 0,
      /* const VkBufferView*           */ .pTexelBufferView = 0,
    };

    vkUpdateDescriptorSets(Device, 1, &DescriptorSetWrite, 0, nullptr);

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo
    {
      /* VkStructureType              */ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      /* const void*                  */ .pNext = nullptr,
      /* VkPipelineLayoutCreateFlags  */ .flags = 0,
      /* uint32_t                     */ .setLayoutCount = 1,
      /* const VkDescriptorSetLayout* */ .pSetLayouts = &PresentDescriptorSetLayout,
      /* uint32_t                     */ .pushConstantRangeCount = 0,
      /* const VkPushConstantRange*   */ .pPushConstantRanges = nullptr,
    };

    utils::AssertResult(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, nullptr, &PresentPipelineLayout));

    VkPipelineVertexInputStateCreateInfo VertexInputState
    {
      /* VkStructureType                          */ .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      /* const void*                              */ .pNext = nullptr,
      /* VkPipelineVertexInputStateCreateFlags    */ .flags = 0,
      /* uint32_t                                 */ .vertexBindingDescriptionCount = 0,
      /* const VkVertexInputBindingDescription*   */ .pVertexBindingDescriptions = nullptr,
      /* uint32_t                                 */ .vertexAttributeDescriptionCount = 0,
      /* const VkVertexInputAttributeDescription* */ .pVertexAttributeDescriptions = nullptr,
    };

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyState
    {
      /* VkStructureType                         */ .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      /* const void*                             */ .pNext = nullptr,
      /* VkPipelineInputAssemblyStateCreateFlags */ .flags = 0,
      /* VkPrimitiveTopology                     */ .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
      /* VkBool32                                */ .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport Viewport { 0, 0, 30, 30, 0, 1 };
    VkRect2D Scissor {{0, 0}, {30, 30}};

    VkPipelineViewportStateCreateInfo ViewportState
    {
      /* VkStructureType                    */ .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      /* const void*                        */ .pNext = nullptr,
      /* VkPipelineViewportStateCreateFlags */ .flags = 0,
      /* uint32_t                           */ .viewportCount = 1,
      /* const VkViewport*                  */ .pViewports = &Viewport,
      /* uint32_t                           */ .scissorCount = 1,
      /* const VkRect2D*                    */ .pScissors = &Scissor,
    };

    VkPipelineRasterizationStateCreateInfo RasterizationState
    {
      /* VkStructureType                         */ .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      /* const void*                             */ .pNext = nullptr,
      /* VkPipelineRasterizationStateCreateFlags */ .flags = 0,
      /* VkBool32                                */ .depthClampEnable = VK_FALSE,
      /* VkBool32                                */ .rasterizerDiscardEnable = VK_FALSE,
      /* VkPolygonMode                           */ .polygonMode = VK_POLYGON_MODE_FILL,
      /* VkCullModeFlags                         */ .cullMode = VK_CULL_MODE_NONE,
      /* VkFrontFace                             */ .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      /* VkBool32                                */ .depthBiasEnable = VK_FALSE,
      /* float                                   */ .depthBiasConstantFactor = 0,
      /* float                                   */ .depthBiasClamp = 0,
      /* float                                   */ .depthBiasSlopeFactor = 0,
      /* float                                   */ .lineWidth = 5.0,
    };

    VkPipelineMultisampleStateCreateInfo MultisampleState
    {
      /* VkStructureType                       */ .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      /* const void*                           */ .pNext = nullptr,
      /* VkPipelineMultisampleStateCreateFlags */ .flags = 0,
      /* VkSampleCountFlagBits                 */ .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      /* VkBool32                              */ .sampleShadingEnable = VK_FALSE,
      /* float                                 */ .minSampleShading = 0,
      /* const VkSampleMask*                   */ .pSampleMask = nullptr,
      /* VkBool32                              */ .alphaToCoverageEnable = VK_FALSE,
      /* VkBool32                              */ .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineDepthStencilStateCreateInfo DepthStencilState
    {
      /* VkStructureType                        */ .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      /* const void*                            */ .pNext = nullptr,
      /* VkPipelineDepthStencilStateCreateFlags */ .flags = 0,
      /* VkBool32                               */ .depthTestEnable = VK_FALSE,
      /* VkBool32                               */ .depthWriteEnable = VK_FALSE,
      /* VkCompareOp                            */ .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
      /* VkBool32                               */ .depthBoundsTestEnable = VK_FALSE,
      /* VkBool32                               */ .stencilTestEnable = VK_FALSE,
      /* VkStencilOpState                       */ .front = VK_STENCIL_OP_KEEP,
      /* VkStencilOpState                       */ .back = VK_STENCIL_OP_KEEP,
      /* float                                  */ .minDepthBounds = 0.0f,
      /* float                                  */ .maxDepthBounds = 1.0f,
    };

    VkPipelineColorBlendAttachmentState ColorBlendAttachmentState
    {
      /* VkBool32              */ .blendEnable = VK_FALSE,
      /* VkBlendFactor         */ .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      /* VkBlendFactor         */ .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      /* VkBlendOp             */ .colorBlendOp = VK_BLEND_OP_ADD,
      /* VkBlendFactor         */ .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      /* VkBlendFactor         */ .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      /* VkBlendOp             */ .alphaBlendOp = VK_BLEND_OP_ADD,
      /* VkColorComponentFlags */ .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo ColorBlendState
    {
      /* VkStructureType                            */ .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      /* const void*                                */ .pNext = nullptr,
      /* VkPipelineColorBlendStateCreateFlags       */ .flags = 0,
      /* VkBool32                                   */ .logicOpEnable = VK_FALSE,
      /* VkLogicOp                                  */ .logicOp = VK_LOGIC_OP_SET,
      /* uint32_t                                   */ .attachmentCount = 1,
      /* const VkPipelineColorBlendAttachmentState* */ .pAttachments = &ColorBlendAttachmentState,
      /* float                                      */ .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkDynamicState DynamicStates[]
    {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo DynamicState
    {
      /* VkStructureType                   */ .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      /* const void*                       */ .pNext = nullptr,
      /* VkPipelineDynamicStateCreateFlags */ .flags = 0,
      /* uint32_t                          */ .dynamicStateCount = (UINT32)std::size(DynamicStates),
      /* const VkDynamicState*             */ .pDynamicStates = DynamicStates,
    };

    shader Shader = LoadShader("bin/shaders/shade");
    std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos = Shader.GetPipelineShaderStageCreateInfos();

    VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo
    {
      /* VkStructureType                               */ .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      /* const void*                                   */ .pNext = nullptr,
      /* VkPipelineCreateFlags                         */ .flags = 0,
      /* uint32_t                                      */ .stageCount = (UINT32)ShaderStageCreateInfos.size(),
      /* const VkPipelineShaderStageCreateInfo*        */ .pStages = ShaderStageCreateInfos.data(),
      /* const VkPipelineVertexInputStateCreateInfo*   */ .pVertexInputState = &VertexInputState,
      /* const VkPipelineInputAssemblyStateCreateInfo* */ .pInputAssemblyState = &InputAssemblyState,
      /* const VkPipelineTessellationStateCreateInfo*  */ .pTessellationState = nullptr,
      /* const VkPipelineViewportStateCreateInfo*      */ .pViewportState = &ViewportState,
      /* const VkPipelineRasterizationStateCreateInfo* */ .pRasterizationState = &RasterizationState,
      /* const VkPipelineMultisampleStateCreateInfo*   */ .pMultisampleState = &MultisampleState,
      /* const VkPipelineDepthStencilStateCreateInfo*  */ .pDepthStencilState = &DepthStencilState,
      /* const VkPipelineColorBlendStateCreateInfo*    */ .pColorBlendState = &ColorBlendState,
      /* const VkPipelineDynamicStateCreateInfo*       */ .pDynamicState = &DynamicState,
      /* VkPipelineLayout                              */ .layout = PresentPipelineLayout,
      /* VkRenderPass                                  */ .renderPass = PresentRenderPass,
      /* uint32_t                                      */ .subpass = 0,
      /* VkPipeline                                    */ .basePipelineHandle = VK_NULL_HANDLE,
      /* int32_t                                       */ .basePipelineIndex = 0,
    };

    utils::AssertResult(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &GraphicsPipelineCreateInfo, nullptr, &PresentPipeline));

    Destroy(Shader);
  } /* InitializePresentPipeline */

  VOID kernel::Resize( VOID )
  {
    vkDeviceWaitIdle(Device);


    for (VkFramebuffer Framebuffer : Framebuffers)
      vkDestroyFramebuffer(Device, Framebuffer, nullptr);
    for (VkImageView ImageView : SwapchainImageViews)
      vkDestroyImageView(Device, ImageView, nullptr);

    VkSwapchainKHR OldSwapchain = Swapchain;

    InitializeSwapchain();
    InitializeFramebuffers();

    Destroy(TargetImage);
    TargetImage = CreateImage(SwapchainImageExtent.width, SwapchainImageExtent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    TransferImageLayout(TargetImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL); // reset image layout

    vkDestroySwapchainKHR(Device, OldSwapchain, nullptr);

    VkDescriptorImageInfo DescriptorImageInfo
    {
      .sampler = TargetImageSampler,
      .imageView = TargetImage.ImageView,
      .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };

    VkWriteDescriptorSet RewriteTargetImage
    {
      /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      /* const void*                   */ .pNext = nullptr,
      /* VkDescriptorSet               */ .dstSet = VK_NULL_HANDLE,
      /* uint32_t                      */ .dstBinding = 0,
      /* uint32_t                      */ .dstArrayElement = 0,
      /* uint32_t                      */ .descriptorCount = 1,
      /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      /* const VkDescriptorImageInfo*  */ .pImageInfo = &DescriptorImageInfo,
      /* const VkDescriptorBufferInfo* */ .pBufferInfo = 0,
      /* const VkBufferView*           */ .pTexelBufferView = 0,
    };

    RewriteTargetImage.dstSet = PresentDescriptorSet;
    vkUpdateDescriptorSets(Device, 1, &RewriteTargetImage, 0, nullptr);

    // Rewrite descriptor sets
    RewriteTargetImage.dstSet = Scene->DescriptorSet;
    vkUpdateDescriptorSets(Device, 1, &RewriteTargetImage, 0, nullptr);

    Camera.SetAspect((FLOAT)SwapchainImageExtent.width, (FLOAT)SwapchainImageExtent.height);
  } /* Resize */
} /* namespace rtt::render */