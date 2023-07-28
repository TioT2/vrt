#ifndef __vrt_render_h_
#define __vrt_render_h_

#include "vrt_mth.h"
#include "vrt_utils.h"

namespace vrt
{
  namespace render
  {
    class core;

    struct buffer
    {
      core *Core = nullptr;

      VkBuffer Buffer = VK_NULL_HANDLE;
      VkDeviceMemory Memory = VK_NULL_HANDLE;
      SIZE_T Size = 0;
      UINT32 MemoryTypeIndex = 0;

      /* constructor */
      buffer( core *Core = nullptr );

      /* !!!ACHTUNG!!! */
      buffer( const buffer &OtherConst );

      /* buffer move assignment */
      buffer & operator=( buffer &&Other ) noexcept;

      /* Get device address of this buffer. */
      VkDeviceAddress GetDeviceAddress( VOID ) const;

      VOID CopyTo( buffer &DestinationBuffer );

      VOID WriteData( VOID *Data, SIZE_T DataSize );

      VOID * MapMemory( VOID );

      VOID UnmapMemory( VOID );

      /* buffer destructor */
      ~buffer( VOID );
    }; /* buffer */

    struct image
    {
      VkImage Image;
      VkDeviceMemory Memory;
      VkFormat Format;
      VkImageView ImageView;
      UINT32 MemoryTypeIndex;
      UINT32 Width, Height;
    }; /* image */

    struct rt_shader
    {
      enum struct module_type
      {
        RAYGEN       = 0,
        ANY_HIT      = 1,
        CLOSEST_HIT  = 2,
        MISS         = 3,
        INTERSECTION = 4,
        CALLABLE     = 5,
      };
      core *Core = nullptr;

      VkShaderModule Raygen       = VK_NULL_HANDLE;
      VkShaderModule AnyHit       = VK_NULL_HANDLE;
      VkShaderModule ClosestHit   = VK_NULL_HANDLE;
      VkShaderModule Miss         = VK_NULL_HANDLE;
      VkShaderModule Intersection = VK_NULL_HANDLE;
      VkShaderModule Callable     = VK_NULL_HANDLE;

      VkShaderModule & GetModule( module_type Type );

      static const CHAR * GetModuleTypeEntryPointName( module_type Type );

      static const CHAR * GetModuleTypeName( module_type Type );
    }; /* rt_shader */

    struct shader
    {
      enum struct module_type
      {
        VERTEX       = 0,
        TESS_CONTROL = 1,
        TESS_EVAL    = 2,
        GEOMETRY     = 3,
        FRAGMENT     = 4,
        COMPUTE      = 5,
      };
      constexpr static SIZE_T ModuleCount = 6;

      VkShaderModule Vertex      = VK_NULL_HANDLE;
      VkShaderModule Fragment    = VK_NULL_HANDLE;
      VkShaderModule Geometry    = VK_NULL_HANDLE;
      VkShaderModule TessEval    = VK_NULL_HANDLE;
      VkShaderModule TessControl = VK_NULL_HANDLE;
      VkShaderModule Compute     = VK_NULL_HANDLE;

      VkShaderModule & GetModule( module_type Type );

      static const CHAR * GetModuleTypeEntryPointName( module_type Type );

      static const CHAR * GetModuleTypeName( module_type Type );

      std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos( VOID );
    }; /* shader */

    class core
    {
    public:
      SDL_Window *Window = nullptr;
      BOOL InstanceSetupProcess = FALSE;

      VkInstance Instance = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;

      VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;

      struct queue_family_indices
      {
        UINT32
          Graphics = UINT32_MAX,
          Present  = UINT32_MAX,
          Transfer = UINT32_MAX,
          Compute  = UINT32_MAX;

        std::vector<UINT32> Unique;
      } QueueFamilies;

      VkDevice Device = VK_NULL_HANDLE;
      VkSurfaceKHR Surface = VK_NULL_HANDLE;

      VkQueue GraphicsQueue = VK_NULL_HANDLE;
      VkQueue PresentQueue = VK_NULL_HANDLE;
      VkQueue TransferQueue = VK_NULL_HANDLE;
      VkQueue ComputeQueue = VK_NULL_HANDLE;

      VkCommandPool GraphicsCommandPool = VK_NULL_HANDLE;
      VkCommandPool TransferCommandPool = VK_NULL_HANDLE;
      VkCommandPool ComputeCommandPool = VK_NULL_HANDLE;

      VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
      VkExtent2D SwapchainImageExtent;
      VkFormat SwapchainImageFormat;
      std::vector<VkImage> SwapchainImages;
      std::vector<VkImageView> SwapchainImageViews;
      std::vector<VkFramebuffer> Framebuffers;

      struct
      {
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR RTPipelineProperties {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR AccelerationStructureProperties {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR, &RTPipelineProperties};
        VkPhysicalDeviceProperties2 Properties2 {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &AccelerationStructureProperties};
        VkPhysicalDeviceProperties &Properties = Properties2.properties;
      } PhysicalDeviceProperties;

      std::vector<const CHAR *> EnabledInstanceLayers
      {
        "VK_LAYER_KHRONOS_validation",
        // PIDORAS!!! "VK_LAYER_RENDERDOC_Capture"
      };

      std::vector<const CHAR *> EnabledInstanceExtensions
      {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
      };

      std::vector<const CHAR *> EnabledDeviceExtensions
      {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_RAY_QUERY_EXTENSION_NAME,
      };

      std::vector<const CHAR *> EnabledDeviceLayers
      {

      };

      /* VulkanAPI provided Debug messenger callback function. */
      static VkBool32 DebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverityFlag, VkDebugUtilsMessageTypeFlagsEXT MessageTypeFlags,
        const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData );

      /* Debug messenger create info fitting for current instance getting function */
      VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo( VOID );

      /* Debug messenger initialization function */
      VOID InitializeDebugMessenger( VOID );

      /* Debug messenger deinitialization function */
      VOID CloseDebugMessenger( VOID );

      VOID InitializeInstance( VOID )
      {
        InstanceSetupProcess = FALSE;
        VkApplicationInfo AppInfo
        {
          /* VkStructureType */ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
          /* const void*     */ .pNext = nullptr,
          /* const char*     */ .pApplicationName = "vrt",
          /* uint32_t        */ .applicationVersion = 0,
          /* const char*     */ .pEngineName = "vrt::engine",
          /* uint32_t        */ .engineVersion = 0,
          /* uint32_t        */ .apiVersion = VK_API_VERSION_1_3,
        };

        // get presentation-required instance extensions from windower
        UINT32 RequiredInstanceExtensionCount = 0, EnabledInstanceExtensionCount = static_cast<UINT32>(EnabledInstanceExtensions.size());
        SDL_Vulkan_GetInstanceExtensions(Window, &RequiredInstanceExtensionCount, nullptr);
        EnabledInstanceExtensions.resize(EnabledInstanceExtensions.size() + RequiredInstanceExtensionCount);
        SDL_Vulkan_GetInstanceExtensions(Window, &RequiredInstanceExtensionCount, &EnabledInstanceExtensions[EnabledInstanceExtensionCount]);

        VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = GetDebugMessengerCreateInfo();

        VkInstanceCreateInfo CreateInfo
        {
          /* VkStructureType          */ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          /* const void*              */ .pNext = &MessengerCreateInfo,
          /* VkInstanceCreateFlags    */ .flags = 0,
          /* const VkApplicationInfo* */ .pApplicationInfo = &AppInfo,
          /* uint32_t                 */ .enabledLayerCount = static_cast<UINT32>(EnabledInstanceLayers.size()),
          /* const char* const*       */ .ppEnabledLayerNames = EnabledInstanceLayers.data(),
          /* uint32_t                 */ .enabledExtensionCount = static_cast<UINT32>(EnabledInstanceExtensions.size()),
          /* const char* const*       */ .ppEnabledExtensionNames = EnabledInstanceExtensions.data(),
        };

        utils::AssertResult(vkCreateInstance(&CreateInfo, nullptr, &Instance), "Error creating instance.");
        volkLoadInstance(Instance);
        InstanceSetupProcess = FALSE;
      } /* InitInstance */

      /* Physical device raiting function */
      UINT32 RatePhysicalDevice( VkPhysicalDevice PhysicalDevice );

      /* Physical device selecting function */
      VOID ChoosePhysicalDevice( VOID );

      /* Device initialization function */
      VOID InitializeDevice( VOID );

      /* Swapchain initialization function */
      VOID InitializeSwapchain( VOID );

      /* Presentation renderpass initialization function */
      VOID InitializePresentRenderPass( VOID );

      /* Framebuffers initialization function */
      VOID InitializeFramebuffers( VOID );

      /* Initialize image for RT rendering to */
      VOID InitializeTargetImage( VOID );

      /* Presentation pipeline initialization function */
      VOID InitializePresentPipeline( VOID );

      VOID InitializeCommandPools( VOID )
      {
        VkCommandPoolCreateInfo CreateInfo
        {
          /* VkStructureType          */ .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          /* const void*              */ .pNext = nullptr,
          /* VkCommandPoolCreateFlags */ .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          /* uint32_t                 */ .queueFamilyIndex = 0,
        };

        struct queue_family_create_description
        {
          UINT32 Index;
          VkCommandPool &CommandPool;
        } Descriptions[]
        {
          {QueueFamilies.Graphics, GraphicsCommandPool},
          {QueueFamilies.Transfer, TransferCommandPool},
          {QueueFamilies.Compute, ComputeCommandPool}
        };

        for (queue_family_create_description &Descr : Descriptions)
        {
          CreateInfo.queueFamilyIndex = Descr.Index;
          utils::AssertResult(vkCreateCommandPool(Device, &CreateInfo, nullptr, &Descr.CommandPool), "error creating command pool");
        }
      } /* InitializeCommandPools */


      image CreateImage( UINT32 Width, UINT32 Height, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlags MemoryProperties )
      {
        VkImageCreateInfo CreateInfo
        {
          /* VkStructureType       */ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
          /* const void*           */ .pNext = nullptr,
          /* VkImageCreateFlags    */ .flags = 0,
          /* VkImageType           */ .imageType = VK_IMAGE_TYPE_2D,
          /* VkFormat              */ .format = Format,
          /* VkExtent3D            */ .extent = VkExtent3D {Width, Height, 1},
          /* uint32_t              */ .mipLevels = 1,
          /* uint32_t              */ .arrayLayers = 1,
          /* VkSampleCountFlagBits */ .samples = VK_SAMPLE_COUNT_1_BIT,
          /* VkImageTiling         */ .tiling = Tiling,
          /* VkImageUsageFlags     */ .usage = UsageFlags,
          /* VkSharingMode         */ // .sharingMode = VK_SHARING_MODE_CONCURRENT,
          /* uint32_t              */ // .queueFamilyIndexCount = static_cast<UINT32>(std::size(QueueFamilies)),
          /* const uint32_t*       */ // .pQueueFamilyIndices = QueueFamilies,
          /* VkImageLayout         */ .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        if (QueueFamilies.Unique.size() != 1)
        {
          CreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
          CreateInfo.queueFamilyIndexCount = static_cast<UINT32>(QueueFamilies.Unique.size());
          CreateInfo.pQueueFamilyIndices = QueueFamilies.Unique.data();
        }
        else
        {
          CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
          CreateInfo.queueFamilyIndexCount = 0;
          CreateInfo.pQueueFamilyIndices = nullptr;
        }

        image Image;

        Image.Format = Format;
        Image.Width = Width;
        Image.Height = Height;
        utils::AssertResult(vkCreateImage(Device, &CreateInfo, nullptr, &Image.Image));

        VkMemoryRequirements MemoryRequirements;
        vkGetImageMemoryRequirements(Device, Image.Image, &MemoryRequirements);

        VkMemoryAllocateInfo AllocInfo
        {
          .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
          .pNext = nullptr,
          .allocationSize = MemoryRequirements.size,
          .memoryTypeIndex = FindMemoryType(MemoryRequirements.memoryTypeBits, MemoryProperties),
        };

        utils::AssertResult(vkAllocateMemory(Device, &AllocInfo, nullptr, &Image.Memory));

        vkBindImageMemory(Device, Image.Image, Image.Memory, 0);

        Image.MemoryTypeIndex = AllocInfo.memoryTypeIndex;
        Image.ImageView = CreateImageView(Image.Image, Image.Format);

        return Image;
      } /* CreateImage */


      VOID ChangeImageLayout( image &Image, VkImageLayout OldLayout, VkImageLayout NewLayout )
      {
        VkCommandBuffer TransferCommandBuffer = BeginTransfer();

        VkImageMemoryBarrier ImageMemoryBarrier
        {
          /* VkStructureType         */ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          /* const void*             */ .pNext = nullptr,
          /* VkAccessFlags           */ .srcAccessMask = 0,
          /* VkAccessFlags           */ .dstAccessMask = 0,
          /* VkImageLayout           */ .oldLayout = OldLayout,
          /* VkImageLayout           */ .newLayout = NewLayout,
          /* uint32_t                */ .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          /* uint32_t                */ .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          /* VkImage                 */ .image = Image.Image,
          /* VkImageSubresourceRange */ .subresourceRange = VkImageSubresourceRange
          {
            /* VkImageAspectFlags */ .aspectMask = (VkImageAspectFlags)(NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
            /* uint32_t           */ .baseMipLevel = 0,
            /* uint32_t           */ .levelCount = 1,
            /* uint32_t           */ .baseArrayLayer = 0,
            /* uint32_t           */ .layerCount = 1,
          },
        };

        VkPipelineStageFlags SourceStageFlags = 0, DestinationStageFlags = 0;

        if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && (NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || NewLayout == VK_IMAGE_LAYOUT_GENERAL))
        {
          ImageMemoryBarrier.srcAccessMask = 0;
          ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

          SourceStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
          DestinationStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
          ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
          ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

          SourceStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
          DestinationStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
        else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
          ImageMemoryBarrier.srcAccessMask = 0;
          ImageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

          SourceStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
          DestinationStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
        else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
          ImageMemoryBarrier.srcAccessMask = 0;
          ImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

          SourceStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
          DestinationStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }

        if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
          ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        vkCmdPipelineBarrier
        (
          TransferCommandBuffer,
          SourceStageFlags, DestinationStageFlags, 0,
          0, nullptr,
          0, nullptr,
          1, &ImageMemoryBarrier
        );

        EndTransfer(TransferCommandBuffer);
      } /* ChangeImageLayout */

      VkImageView CreateImageView( VkImage Image, VkFormat ImageFormat )
      {
        VkImageViewCreateInfo CreateInfo
        {
          /* VkStructureType         */ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          /* const void*             */ .pNext = nullptr,
          /* VkImageViewCreateFlags  */ .flags = 0,
          /* VkImage                 */ .image = Image,
          /* VkImageViewType         */ .viewType = VK_IMAGE_VIEW_TYPE_2D,
          /* VkFormat                */ .format = ImageFormat,
          /* VkComponentMapping      */ .components = VkComponentMapping { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
          /* VkImageSubresourceRange */ .subresourceRange = VkImageSubresourceRange
          {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
          },
        };

        VkImageView ImageView = VK_NULL_HANDLE;
        utils::AssertResult(vkCreateImageView(Device, &CreateInfo, nullptr, &ImageView), "error creating image view");
        return ImageView;
      } /* CreateImageView */

      VkCommandBuffer CreateCommandBuffer( VkCommandPool CommandPool )
      {
        VkCommandBufferAllocateInfo AllocInfo
        {
          /* VkStructureType      */ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          /* const void*          */ .pNext = nullptr,
          /* VkCommandPool        */ .commandPool = CommandPool,
          /* VkCommandBufferLevel */ .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          /* uint32_t             */ .commandBufferCount = 1,
        };

        VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
        utils::AssertResult(vkAllocateCommandBuffers(Device, &AllocInfo, &CommandBuffer), "error allocating command buffers");
        return CommandBuffer;
      } /* CreateCommandBuffer */

      UINT32 FindMemoryType( UINT32 TypeFilter, VkMemoryPropertyFlags PropertyFlags )
      {
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties {};
        vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &physicalDeviceMemoryProperties);

        for (UINT32 i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
        {
          if (TypeFilter & (1 << i) && utils::CheckFlags(physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags, PropertyFlags))
            return i;
        }

        return UINT32_MAX;
      } /* findMemoryType */

      buffer CreateBuffer( VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemoryPropertyFlags )
      {
        buffer Buffer {this};

        VkBufferCreateInfo CreateInfo
        {
          .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .size = Size,
          .usage = Usage,
        };

        if (QueueFamilies.Unique.size() == 1)
        {
          CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
          CreateInfo.queueFamilyIndexCount = 0;
          CreateInfo.pQueueFamilyIndices = nullptr;
        }
        else
        {
          CreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
          CreateInfo.queueFamilyIndexCount = static_cast<UINT32>(QueueFamilies.Unique.size());
          CreateInfo.pQueueFamilyIndices = QueueFamilies.Unique.data();
        }

        utils::AssertResult(vkCreateBuffer(Device, &CreateInfo, nullptr, &Buffer.Buffer), "error creating buffer");

        VkMemoryRequirements MemoryRequirements;
        vkGetBufferMemoryRequirements(Device, Buffer.Buffer, &MemoryRequirements);
        Buffer.MemoryTypeIndex = FindMemoryType(MemoryRequirements.memoryTypeBits, MemoryPropertyFlags);
        Buffer.Size = MemoryRequirements.size;

        // VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT 
        VkMemoryAllocateFlagsInfo AllocateFlagsInfo
        {
          .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
          .pNext = nullptr,
          .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
          .deviceMask = 0,
        };

        VkMemoryAllocateInfo AllocateInfo
        {
          .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
          .pNext = &AllocateFlagsInfo,
          .allocationSize = Buffer.Size,
          .memoryTypeIndex = Buffer.MemoryTypeIndex,
        };

        utils::AssertResult(vkAllocateMemory(Device, &AllocateInfo, nullptr, &Buffer.Memory), "error allocating memory");

        vkBindBufferMemory(Device, Buffer.Buffer, Buffer.Memory, 0);

        return std::move(Buffer);
      } /* CreateBuffer */




      VkCommandBuffer BeginSingleTimeCommands( VOID )
      {
        VkCommandBuffer CommandBuffer = CreateCommandBuffer(GraphicsCommandPool);

        VkCommandBufferBeginInfo BeginInfo
        {
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
          .pNext = nullptr,
          .flags = 0,
          .pInheritanceInfo = nullptr,
        };

        vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

        return CommandBuffer;
      } /* BeginSingleTimeCommands */

      VOID EndSingleTimeCommands( VkCommandBuffer SingleTimeCommandBuffer )
      {
        vkEndCommandBuffer(SingleTimeCommandBuffer);

        VkSubmitInfo SubmitInfo
        {
          /* VkStructureType             */ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          /* const void*                 */ .pNext = nullptr,
          /* uint32_t                    */ .waitSemaphoreCount = 0,
          /* const VkSemaphore*          */ .pWaitSemaphores = nullptr,
          /* const VkPipelineStageFlags* */ .pWaitDstStageMask = nullptr,
          /* uint32_t                    */ .commandBufferCount = 1,
          /* const VkCommandBuffer*      */ .pCommandBuffers = &SingleTimeCommandBuffer,
          /* uint32_t                    */ .signalSemaphoreCount = 0,
          /* const VkSemaphore*          */ .pSignalSemaphores = nullptr,
        };

        vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);

        vkQueueWaitIdle(GraphicsQueue);

        vkFreeCommandBuffers(Device, GraphicsCommandPool, 1, &SingleTimeCommandBuffer);
      } /* EndSingleTimeCommands */

      VkCommandBuffer BeginTransfer( VOID )
      {
        VkCommandBuffer CommandBuffer = CreateCommandBuffer(TransferCommandPool);

        VkCommandBufferBeginInfo BeginInfo
        {
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
          .pNext = nullptr,
          .flags = 0,
          .pInheritanceInfo = nullptr,
        };

        vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

        return CommandBuffer;
      } /* BeginTransfer */

      VOID EndTransfer( VkCommandBuffer CommandBuffer )
      {
        vkEndCommandBuffer(CommandBuffer);

        VkSubmitInfo SubmitInfo
        {
          /* VkStructureType             */ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          /* const void*                 */ .pNext = nullptr,
          /* uint32_t                    */ .waitSemaphoreCount = 0,
          /* const VkSemaphore*          */ .pWaitSemaphores = nullptr,
          /* const VkPipelineStageFlags* */ .pWaitDstStageMask = nullptr,
          /* uint32_t                    */ .commandBufferCount = 1,
          /* const VkCommandBuffer*      */ .pCommandBuffers = &CommandBuffer,
          /* uint32_t                    */ .signalSemaphoreCount = 0,
          /* const VkSemaphore*          */ .pSignalSemaphores = nullptr,
        };

        vkQueueSubmit(TransferQueue, 1, &SubmitInfo, VK_NULL_HANDLE);

        vkQueueWaitIdle(TransferQueue);

        vkFreeCommandBuffers(Device, TransferCommandPool, 1, &CommandBuffer);
      } /* EndTransfer */

      rt_shader LoadRTShader( std::string_view Name );

      shader LoadShader( std::string_view Name );

      VOID Destroy( rt_shader &Shader )
      {
        if (Shader.Raygen       != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Raygen,       nullptr);
        if (Shader.AnyHit       != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.AnyHit,       nullptr);
        if (Shader.ClosestHit   != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.ClosestHit,   nullptr);
        if (Shader.Miss         != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Miss,         nullptr);
        if (Shader.Intersection != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Intersection, nullptr);
        if (Shader.Callable     != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Callable,     nullptr);
      } /* Destroy */

      VOID Destroy( shader &Shader )
      {
        if (Shader.Vertex      != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Vertex,      nullptr);
        if (Shader.Fragment    != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Fragment,    nullptr);
        if (Shader.Geometry    != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Geometry,    nullptr);
        if (Shader.TessEval    != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.TessEval,    nullptr);
        if (Shader.TessControl != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.TessControl, nullptr);
        if (Shader.Compute     != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Compute,     nullptr);
      } /* Destroy */

      VkPipeline Pipeline = VK_NULL_HANDLE;
      VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
      VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
      VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
      VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;

      buffer VertexBuffer;
      buffer BLASStorageBuffer;
      VkAccelerationStructureKHR BLAS = VK_NULL_HANDLE;

      buffer InstanceBuffer;
      buffer TLASStorageBuffer;
      VkAccelerationStructureKHR TLAS = VK_NULL_HANDLE;

      buffer SBTStorageBuffer = VK_NULL_HANDLE;
      VkCommandBuffer GraphicsCommandBuffer = VK_NULL_HANDLE;
      VkFence InFlightFence = VK_NULL_HANDLE;
      VkSemaphore ImageAvailableSemaphore = VK_NULL_HANDLE, RenderFinishedSemaphore = VK_NULL_HANDLE;
      UINT32 SBTAlignedGroupSize = 0;

      buffer TestBuffer;

      image TargetImage;
      VkSampler TargetImageSampler = VK_NULL_HANDLE;
      VkRenderPass PresentRenderPass = VK_NULL_HANDLE;
      VkDescriptorSetLayout PresentDescriptorSetLayout = VK_NULL_HANDLE;
      VkDescriptorSet PresentDescriptorSet = VK_NULL_HANDLE;
      VkDescriptorPool PresentDescriptorPool = VK_NULL_HANDLE;
      VkPipelineLayout PresentPipelineLayout = VK_NULL_HANDLE;
      VkPipeline PresentPipeline = VK_NULL_HANDLE;


      VOID TestInitPrimitiveData( VOID )
      {
        /* Initialize triangle */
        FLOAT Vtx[]
        {
          0.25f, 0.25f, 0.0f,
          0.75f, 0.25f, 0.0f,
          0.50f, 0.75f, 0.0f
        };

        VertexBuffer = CreateBuffer(sizeof(Vtx),
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VertexBuffer.WriteData(Vtx, sizeof(Vtx));
      } /* TestInitPrimitiveData */

      VOID TestInitBLAS( VOID )
      {
        VkDeviceAddress VertexBufferAddress = VertexBuffer.GetDeviceAddress();

        VkAccelerationStructureGeometryTrianglesDataKHR TrianglesData
        {
          /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
          /* const void*                   */ .pNext = nullptr,
          /* VkFormat                      */ .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
          /* VkDeviceOrHostAddressConstKHR */ .vertexData = { .deviceAddress = VertexBuffer.GetDeviceAddress() },
          /* VkDeviceSize                  */ .vertexStride = sizeof(FLOAT) * 3,
          /* uint32_t                      */ .maxVertex = 3,
          /* VkIndexType                   */ .indexType = VK_INDEX_TYPE_NONE_KHR,
          /* VkDeviceOrHostAddressConstKHR */ .indexData = {},
          /* VkDeviceOrHostAddressConstKHR */ .transformData = {},
        };

        VkAccelerationStructureGeometryKHR Geometry
        {
          /* VkStructureType                        */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
          /* const void*                            */ .pNext = nullptr,
          /* VkGeometryTypeKHR                      */ .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
          /* VkAccelerationStructureGeometryDataKHR */ .geometry = TrianglesData,
          /* VkGeometryFlagsKHR                     */ .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
        };

        VkAccelerationStructureBuildRangeInfoKHR RangeInfo
        {
          .primitiveCount = 1,
          .primitiveOffset = 0,
          .firstVertex = 0,
          .transformOffset = 0,
        };

        VkAccelerationStructureBuildGeometryInfoKHR GeometryBuildInfo
        {
          /* VkStructureType                                  */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
          /* const void*                                      */ .pNext = nullptr,
          /* VkAccelerationStructureTypeKHR                   */ .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
          /* VkBuildAccelerationStructureFlagsKHR             */ .flags = 0,
          /* VkBuildAccelerationStructureModeKHR              */ .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
          /* VkAccelerationStructureKHR                       */ .srcAccelerationStructure = VK_NULL_HANDLE,
          /* VkAccelerationStructureKHR                       */ .dstAccelerationStructure = VK_NULL_HANDLE,
          /* uint32_t                                         */ .geometryCount = 1,
          /* const VkAccelerationStructureGeometryKHR*        */ .pGeometries = &Geometry,
          /* const VkAccelerationStructureGeometryKHR* const* */ .ppGeometries = nullptr,
          /* VkDeviceOrHostAddressKHR                         */ .scratchData = 0,
        };

        
        VkAccelerationStructureBuildSizesInfoKHR BuildSizesInfo {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};

        vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &GeometryBuildInfo, &RangeInfo.primitiveCount, &BuildSizesInfo);

        BLASStorageBuffer = CreateBuffer(BuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkAccelerationStructureCreateInfoKHR AccelerationStructureCreateInfo
        {
          /* VkStructureType                       */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
          /* const void*                           */ .pNext = nullptr,
          /* VkAccelerationStructureCreateFlagsKHR */ .createFlags = 0,
          /* VkBuffer                              */ .buffer = BLASStorageBuffer.Buffer,
          /* VkDeviceSize                          */ .offset = 0,
          /* VkDeviceSize                          */ .size = BuildSizesInfo.accelerationStructureSize,
          /* VkAccelerationStructureTypeKHR        */ .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
          /* VkDeviceAddress                       */ .deviceAddress = 0,
        };

        vkCreateAccelerationStructureKHR(Device, &AccelerationStructureCreateInfo, nullptr, &BLAS);

        GeometryBuildInfo.dstAccelerationStructure = BLAS;
        buffer ScratchBuffer = CreateBuffer(BuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        GeometryBuildInfo.scratchData.deviceAddress = ScratchBuffer.GetDeviceAddress();

        VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();
        VkAccelerationStructureBuildRangeInfoKHR *PtrRangeInfo = &RangeInfo;
        vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &GeometryBuildInfo, &PtrRangeInfo);
        EndSingleTimeCommands(CommandBuffer);
      } /* TestInitBLAS */

      VOID TestInitTLAS( VOID )
      {
        VkAccelerationStructureDeviceAddressInfoKHR DeviceAddressInfo
        {
          .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
          .pNext = nullptr,
          .accelerationStructure = BLAS,
        };

        VkDeviceAddress BLASAddres = vkGetAccelerationStructureDeviceAddressKHR(Device, &DeviceAddressInfo);

        VkAccelerationStructureInstanceKHR AccelerationStructureInstance
        {
          .transform = VkTransformMatrixKHR
          {{
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
          }},
          .instanceCustomIndex = 0,
          .mask = 0xFF,
          .instanceShaderBindingTableRecordOffset = 0,
          .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
          .accelerationStructureReference = BLASAddres,
        };

        InstanceBuffer = CreateBuffer
        (
          sizeof(VkAccelerationStructureInstanceKHR),
          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        VkAccelerationStructureInstanceKHR *InstanceData;
        vkMapMemory(Device, InstanceBuffer.Memory, 0, InstanceBuffer.Size, 0, reinterpret_cast<VOID **>(&InstanceData));
        *InstanceData = VkAccelerationStructureInstanceKHR
        {
          .transform = VkTransformMatrixKHR
          {{
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
          }},
          .instanceCustomIndex = 0,
          .mask = 0xFF,
          .instanceShaderBindingTableRecordOffset = 0,
          .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
          .accelerationStructureReference = BLASAddres,
        };
        vkUnmapMemory(Device, InstanceBuffer.Memory);

        VkAccelerationStructureBuildRangeInfoKHR RangeInfo
        {
          .primitiveCount = 1,
          .primitiveOffset = 0,
          .firstVertex = 0,
          .transformOffset = 0,
        };

        VkAccelerationStructureGeometryInstancesDataKHR InstancesVk
        {
          .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
          .pNext = nullptr,
          .arrayOfPointers = VK_FALSE,
          .data = { .deviceAddress = InstanceBuffer.GetDeviceAddress() },
        };

        VkAccelerationStructureGeometryKHR Geometry
        {
          .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
          .pNext = nullptr,
          .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
          .geometry = { .instances = InstancesVk },
          .flags = 0,
        };

        VkAccelerationStructureBuildGeometryInfoKHR BuildGeometryInfo
        {
          /* VkStructureType                                  */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
          /* const void*                                      */ .pNext = nullptr,
          /* VkAccelerationStructureTypeKHR                   */ .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
          /* VkBuildAccelerationStructureFlagsKHR             */ .flags = 0,
          /* VkBuildAccelerationStructureModeKHR              */ .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
          /* VkAccelerationStructureKHR                       */ .srcAccelerationStructure = VK_NULL_HANDLE,
          /* VkAccelerationStructureKHR                       */ .dstAccelerationStructure = TLAS,
          /* uint32_t                                         */ .geometryCount = 1,
          /* const VkAccelerationStructureGeometryKHR*        */ .pGeometries = &Geometry,
          /* const VkAccelerationStructureGeometryKHR* const* */ .ppGeometries = nullptr,
          /* VkDeviceOrHostAddressKHR                         */ .scratchData = {},
        };

        VkAccelerationStructureBuildSizesInfoKHR BuildSizesInfo {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};

        vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &BuildGeometryInfo, &RangeInfo.primitiveCount, &BuildSizesInfo);

        TLASStorageBuffer = CreateBuffer(BuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkAccelerationStructureCreateInfoKHR CreateInfo
        {
          .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
          .pNext = nullptr,
          .createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
          .buffer = TLASStorageBuffer.Buffer,
          .offset = 0,
          .size = BuildSizesInfo.accelerationStructureSize,
          .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
          .deviceAddress = TLASStorageBuffer.GetDeviceAddress(),
        };

        utils::AssertResult(vkCreateAccelerationStructureKHR(Device, &CreateInfo, nullptr, &TLAS), "error building TLAS.");

        BuildGeometryInfo.dstAccelerationStructure = TLAS;

        buffer ScratchBuffer = CreateBuffer(BuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        BuildGeometryInfo.scratchData.deviceAddress = ScratchBuffer.GetDeviceAddress();

        VkAccelerationStructureBuildRangeInfoKHR *PtrRangeInfo = &RangeInfo;

        VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();
        vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &BuildGeometryInfo, &PtrRangeInfo);
        EndSingleTimeCommands(CommandBuffer);
      } /* TestInitTLAS */

      VkShaderModule CreateShaderModule( std::span<const UINT32> SPIRVCode )
      {
        VkShaderModuleCreateInfo CreateInfo
        {
          .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .codeSize = static_cast<UINT32>(SPIRVCode.size()),
          .pCode = SPIRVCode.data(),
        };

        VkShaderModule ShaderModule = VK_NULL_HANDLE;
        VkResult Result = vkCreateShaderModule(Device, &CreateInfo, nullptr, &ShaderModule);

        return Result == VK_SUCCESS ? ShaderModule : VK_NULL_HANDLE;
      } /* CreateInfo */

      VOID TestInitPipeline( VOID )
      {
        TestBuffer = CreateBuffer(sizeof(FLOAT) * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[]
        {
          {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
          },
          {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
          },
          {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
          },
        };

        VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo
        {
          /* VkStructureType                     */ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
          /* const void*                         */ .pNext = nullptr,
          /* VkDescriptorSetLayoutCreateFlags    */ .flags = 0,
          /* uint32_t                            */ .bindingCount = static_cast<UINT32>(std::size(DescriptorSetLayoutBindings)),
          /* const VkDescriptorSetLayoutBinding* */ .pBindings = DescriptorSetLayoutBindings,
        };

        utils::AssertResult(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, nullptr, &DescriptorSetLayout), "error creating descriptor set layout.");

        VkDescriptorPoolSize PoolSizes[]
        {
          { .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,              .descriptorCount = 1, },
          { .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1, },
          { .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             .descriptorCount = 1, }
        };

        VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo
        {
          /* VkStructureType             */ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
          /* const void*                 */ .pNext = nullptr,
          /* VkDescriptorPoolCreateFlags */ .flags = 0,
          /* uint32_t                    */ .maxSets = 1,
          /* uint32_t                    */ .poolSizeCount = static_cast<UINT32>(std::size(PoolSizes)),
          /* const VkDescriptorPoolSize* */ .pPoolSizes = PoolSizes,
        };

        utils::AssertResult(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, nullptr, &DescriptorPool), "error creating descriptor pool.");

        VkDescriptorSetAllocateInfo DescriptorSetAllocInfo
        {
          .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
          .pNext = nullptr,
          .descriptorPool = DescriptorPool,
          .descriptorSetCount = 1,
          .pSetLayouts = &DescriptorSetLayout,
        };

        utils::AssertResult(vkAllocateDescriptorSets(Device, &DescriptorSetAllocInfo, &DescriptorSet));

        VkWriteDescriptorSetAccelerationStructureKHR DescriptorSetAccelerationStructure
        {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
          .pNext = nullptr,
          .accelerationStructureCount = 1,
          .pAccelerationStructures = &TLAS,
        };
        VkDescriptorBufferInfo StorageBufferBindingInfo
        {
          .buffer = TestBuffer.Buffer,
          .offset = 0,
          .range = TestBuffer.Size,
        };
        VkDescriptorImageInfo DescriptorSetWriteImageInfo
        {
          .sampler = TargetImageSampler,
          .imageView = TargetImage.ImageView,
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        VkWriteDescriptorSet DescriptorSetWrites[]
        {
          /* Target binding */
          {
            /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            /* const void*                   */ .pNext = nullptr,
            /* VkDescriptorSet               */ .dstSet = DescriptorSet,
            /* uint32_t                      */ .dstBinding = 0,
            /* uint32_t                      */ .dstArrayElement = 0,
            /* uint32_t                      */ .descriptorCount = 1,
            /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            /* const VkDescriptorImageInfo*  */ .pImageInfo = &DescriptorSetWriteImageInfo,
            /* const VkDescriptorBufferInfo* */ .pBufferInfo = nullptr,
            /* const VkBufferView*           */ .pTexelBufferView = nullptr,
          },
          /* TLAS binding */
          {
            /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            /* const void*                   */ .pNext = &DescriptorSetAccelerationStructure,
            /* VkDescriptorSet               */ .dstSet = DescriptorSet,
            /* uint32_t                      */ .dstBinding = 1,
            /* uint32_t                      */ .dstArrayElement = 0,
            /* uint32_t                      */ .descriptorCount = 1,
            /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
            /* const VkDescriptorImageInfo*  */ .pImageInfo = 0,
            /* const VkDescriptorBufferInfo* */ .pBufferInfo = 0,
            /* const VkBufferView*           */ .pTexelBufferView = 0,
          },
          {
            /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            /* const void*                   */ .pNext = &DescriptorSetAccelerationStructure,
            /* VkDescriptorSet               */ .dstSet = DescriptorSet,
            /* uint32_t                      */ .dstBinding = 2,
            /* uint32_t                      */ .dstArrayElement = 0,
            /* uint32_t                      */ .descriptorCount = 1,
            /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            /* const VkDescriptorImageInfo*  */ .pImageInfo = 0,
            /* const VkDescriptorBufferInfo* */ .pBufferInfo = &StorageBufferBindingInfo,
            /* const VkBufferView*           */ .pTexelBufferView = 0,
          }
        };

        vkUpdateDescriptorSets(Device, static_cast<UINT32>(std::size(DescriptorSetWrites)), DescriptorSetWrites, 0, nullptr);

        rt_shader Shader = LoadRTShader("bin/shaders/triangle");

        std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos
        {
          {
            /* VkStructureType                  */ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            /* VkShaderStageFlagBits            */ .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            /* VkShaderModule                   */ .module = Shader.Raygen,
            /* const char*                      */ .pName = rt_shader::GetModuleTypeEntryPointName(rt_shader::module_type::RAYGEN),
          },
          {
            /* VkStructureType                  */ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            /* VkShaderStageFlagBits            */ .stage = VK_SHADER_STAGE_MISS_BIT_KHR,
            /* VkShaderModule                   */ .module = Shader.Miss,
            /* const char*                      */ .pName = rt_shader::GetModuleTypeEntryPointName(rt_shader::module_type::MISS)
          },
          {
            /* VkStructureType                  */ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            /* VkShaderStageFlagBits            */ .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
            /* VkShaderModule                   */ .module = Shader.ClosestHit,
            /* const char*                      */ .pName = rt_shader::GetModuleTypeEntryPointName(rt_shader::module_type::CLOSEST_HIT),
          },
        };
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> ShaderGroupCreateInfos
        {
          {
            .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
            .pNext = nullptr,
            .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
            .generalShader = 0,
            .closestHitShader = VK_SHADER_UNUSED_KHR,
            .anyHitShader = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR,
            .pShaderGroupCaptureReplayHandle = VK_NULL_HANDLE,
          },
          {
            .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
            .pNext = nullptr,
            .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
            .generalShader = 1,
            .closestHitShader = VK_SHADER_UNUSED_KHR,
            .anyHitShader = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR,
            .pShaderGroupCaptureReplayHandle = VK_NULL_HANDLE,
          },
          {
            .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
            .pNext = nullptr,
            .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
            .generalShader = VK_SHADER_UNUSED_KHR,
            .closestHitShader = 2,
            .anyHitShader = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR,
            .pShaderGroupCaptureReplayHandle = VK_NULL_HANDLE,
          },
        };

        VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo
        {
          /* VkStructureType              */ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
          /* const void*                  */ .pNext = nullptr,
          /* VkPipelineLayoutCreateFlags  */ .flags = 0,
          /* uint32_t                     */ .setLayoutCount = 1,
          /* const VkDescriptorSetLayout* */ .pSetLayouts = &DescriptorSetLayout,
          /* uint32_t                     */ .pushConstantRangeCount = 0,
          /* const VkPushConstantRange*   */ .pPushConstantRanges = nullptr,
        };

        utils::AssertResult(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, nullptr, &PipelineLayout), "Pipeline layout");

        VkRayTracingPipelineCreateInfoKHR PipelineCreateInfo
        {
          /* VkStructureType                                   */ .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
          /* const void*                                       */ .pNext = nullptr,
          /* VkPipelineCreateFlags                             */ .flags = 0,
          /* uint32_t                                          */ .stageCount = static_cast<UINT32>(ShaderStageCreateInfos.size()),
          /* const VkPipelineShaderStageCreateInfo*            */ .pStages = ShaderStageCreateInfos.data(),
          /* uint32_t                                          */ .groupCount = static_cast<UINT32>(ShaderGroupCreateInfos.size()),
          /* const VkRayTracingShaderGroupCreateInfoKHR*       */ .pGroups = ShaderGroupCreateInfos.data(),
          /* uint32_t                                          */ .maxPipelineRayRecursionDepth = 1,
          /* const VkPipelineLibraryCreateInfoKHR*             */ .pLibraryInfo = VK_NULL_HANDLE,
          /* const VkRayTracingPipelineInterfaceCreateInfoKHR* */ .pLibraryInterface = VK_NULL_HANDLE,
          /* const VkPipelineDynamicStateCreateInfo*           */ .pDynamicState = nullptr,
          /* VkPipelineLayout                                  */ .layout = PipelineLayout,
          /* VkPipeline                                        */ .basePipelineHandle = VK_NULL_HANDLE,
          /* int32_t                                           */ .basePipelineIndex = 0,
        };

        utils::AssertResult(vkCreateRayTracingPipelinesKHR(Device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &Pipeline), "error creating RT pipelines");

        Destroy(Shader);

        SIZE_T GroupCount = ShaderGroupCreateInfos.size();
        SIZE_T GroupHandleSize = PhysicalDeviceProperties.RTPipelineProperties.shaderGroupHandleSize;

        SIZE_T AlignedGroupSize = utils::Align(GroupHandleSize, PhysicalDeviceProperties.RTPipelineProperties.shaderGroupBaseAlignment);
        SBTAlignedGroupSize = AlignedGroupSize;

        SIZE_T SBTSize = GroupCount * AlignedGroupSize;

        std::vector<BYTE> ShaderHandleStorage;
        ShaderHandleStorage.resize(SBTSize);

        utils::AssertResult(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipeline, 0, GroupCount, SBTSize, ShaderHandleStorage.data()));

        SBTStorageBuffer = CreateBuffer(SBTSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        BYTE *Data = reinterpret_cast<BYTE *>(SBTStorageBuffer.MapMemory());
        for (SIZE_T g = 0; g < GroupCount; g++)
        {
          std::memcpy(Data, ShaderHandleStorage.data() + g * GroupHandleSize, GroupHandleSize);
          Data += AlignedGroupSize;
        }
        SBTStorageBuffer.UnmapMemory();
      } /* TestInitPipeline */

      VOID Initialize( SDL_Window *RenderWindow )
      {
        Window = RenderWindow;

        volkInitialize();
        InitializeInstance();
        InitializeDebugMessenger();
        utils::Assert(SDL_Vulkan_CreateSurface(Window, Instance, &Surface) == SDL_TRUE, "error initializing surface");
        InitializeDevice();
        InitializeCommandPools();

        InitializeSwapchain();
        InitializePresentRenderPass();
        InitializeFramebuffers();
        InitializeTargetImage();
        InitializePresentPipeline();


        // First-triangle functions
        TestInitPrimitiveData();
        TestInitBLAS();
        TestInitTLAS();
        TestInitPipeline();

        VkFenceCreateInfo FenceCreateInfo
        {
          .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
          .pNext = nullptr,
          .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        utils::AssertResult(vkCreateFence(Device, &FenceCreateInfo, nullptr, &InFlightFence));

        VkSemaphoreCreateInfo SemaphoreCreateInfo
        {
          .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
        };

        utils::AssertResult(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &ImageAvailableSemaphore));
        utils::AssertResult(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderFinishedSemaphore));

        GraphicsCommandBuffer = CreateCommandBuffer(GraphicsCommandPool);
      } /* Initialize */

      VOID Render( VOID )
      {
        VOID *TestDataView = TestBuffer.MapMemory();
        std::cout << *(FLOAT *)TestDataView << "\n";
        TestBuffer.UnmapMemory();

        // wait for rendering ended
        VkFence RenderingFences[] {InFlightFence};
        vkWaitForFences(Device, 1, &InFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(Device, 1, &InFlightFence);

        // recreate frame
        UINT32 ImageIndex = UINT32_MAX;
        VkResult result = vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, ImageAvailableSemaphore, VK_NULL_HANDLE, &ImageIndex);

        if (result != VK_SUCCESS)
          return;


        vkResetCommandBuffer(GraphicsCommandBuffer, 0);

        /* Write output image to descriptor sets */

        FLOAT *TestData = reinterpret_cast<FLOAT *>(TestBuffer.MapMemory());
        std::cout << *TestData << "\n";
        *TestData = 0;
        TestBuffer.UnmapMemory();

        VkCommandBufferBeginInfo CommandBufferBeginInfo
        {
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
          .pNext = nullptr,
          .flags = 0,
          .pInheritanceInfo = nullptr,
        };


        vkBeginCommandBuffer(GraphicsCommandBuffer, &CommandBufferBeginInfo);

        vkCmdBindPipeline(GraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline);
        vkCmdBindDescriptorSets(GraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, PipelineLayout, 0, 1, &DescriptorSet, 0, nullptr);

        VkStridedDeviceAddressRegionKHR RayGenRegion
        {
          .deviceAddress = SBTStorageBuffer.GetDeviceAddress() + SBTAlignedGroupSize * 0,
          .stride = SBTAlignedGroupSize,
          .size = SBTAlignedGroupSize,
        };
        VkStridedDeviceAddressRegionKHR MissRegion
        {
          .deviceAddress = SBTStorageBuffer.GetDeviceAddress() + SBTAlignedGroupSize * 1,
          .stride = SBTAlignedGroupSize,
          .size = SBTAlignedGroupSize,
        };
        VkStridedDeviceAddressRegionKHR HitRegion
        {
          .deviceAddress = SBTStorageBuffer.GetDeviceAddress() + SBTAlignedGroupSize * 2,
          .size = SBTAlignedGroupSize
        };

        VkStridedDeviceAddressRegionKHR CallableRegion {};

        vkCmdTraceRaysKHR
        (
          GraphicsCommandBuffer,
          &RayGenRegion,
          &MissRegion,
          &HitRegion,
          &CallableRegion,
          SwapchainImageExtent.width, SwapchainImageExtent.height, 1
        );

        VkMemoryBarrier MemoryBarrier
        {
          .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
          .pNext = nullptr,
          .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
          .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        };
        vkCmdPipelineBarrier(GraphicsCommandBuffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &MemoryBarrier, 0, nullptr, 0, nullptr);

        VkClearValue ClearValue { 0.0f, 1.0f, 0.0f, 1.0f };

        VkRenderPassBeginInfo BeginInfo
        {
          /* VkStructureType     */ .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
          /* const void*         */ .pNext = nullptr,
          /* VkRenderPass        */ .renderPass = PresentRenderPass,
          /* VkFramebuffer       */ .framebuffer = Framebuffers[ImageIndex],
          /* VkRect2D            */ .renderArea = { 0, 0, SwapchainImageExtent.width, SwapchainImageExtent.height },
          /* uint32_t            */ .clearValueCount = 1,
          /* const VkClearValue* */ .pClearValues = &ClearValue,
        };
        vkCmdBeginRenderPass(GraphicsCommandBuffer, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(GraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PresentPipeline);

        VkViewport Viewport { 0, 0, SwapchainImageExtent.width, SwapchainImageExtent.height, 0.0f, 1.0f };
        vkCmdSetViewport(GraphicsCommandBuffer, 0, 1, &Viewport);
        VkRect2D Scissor { {0, 0}, SwapchainImageExtent };
        vkCmdSetScissor(GraphicsCommandBuffer, 0, 1, &Scissor);
        vkCmdBindDescriptorSets(GraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PresentPipelineLayout, 0, 1, &PresentDescriptorSet, 0, nullptr);
        vkCmdDraw(GraphicsCommandBuffer, 4, 1, 0, 0);

        vkCmdEndRenderPass(GraphicsCommandBuffer);

        vkEndCommandBuffer(GraphicsCommandBuffer);

        // submit commands to queue
        VkSemaphore waitSemaphores[] {ImageAvailableSemaphore};
        VkPipelineStageFlags Flags[] {VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] {RenderFinishedSemaphore};

        VkSubmitInfo GraphicsSubmitInfo
        {
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .pNext = nullptr,
          .waitSemaphoreCount = static_cast<UINT32>(std::size(waitSemaphores)),
          .pWaitSemaphores = waitSemaphores,
          .pWaitDstStageMask = Flags,
          .commandBufferCount = 1,
          .pCommandBuffers = &GraphicsCommandBuffer,
          .signalSemaphoreCount = (UINT32)std::size(signalSemaphores),
          .pSignalSemaphores = signalSemaphores,
        };

        utils::AssertResult(vkQueueSubmit(GraphicsQueue, 1, &GraphicsSubmitInfo, InFlightFence), "can't submit graphics command buffer");

        VkSwapchainKHR swapchains[] {Swapchain};
        VkPresentInfoKHR presentInfo
        {
          .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
          .pNext = nullptr,
          .waitSemaphoreCount = (UINT32)std::size(signalSemaphores),
          .pWaitSemaphores = signalSemaphores,
          .swapchainCount = (UINT32)std::size(swapchains),
          .pSwapchains = swapchains,
          .pImageIndices = &ImageIndex,
          .pResults = nullptr,
        };

        vkQueuePresentKHR(PresentQueue, &presentInfo);
      } /* Render */

      VOID Close( VOID )
      {
        vkDeviceWaitIdle(Device);

        VkCommandPool CommandPools[] {GraphicsCommandPool, TransferCommandPool, ComputeCommandPool};
        for (VkCommandPool CommandPool : CommandPools)
          vkDestroyCommandPool(Device, CommandPool, nullptr);

        for (VkImageView ImageView : SwapchainImageViews)
          vkDestroyImageView(Device, ImageView, nullptr);
        vkDestroySwapchainKHR(Device, Swapchain, nullptr);
        vkDestroyDevice(Device, nullptr);
        vkDestroySurfaceKHR(Instance, Surface, nullptr);
        CloseDebugMessenger();
        vkDestroyInstance(Instance, nullptr);
      } /* Close */
    }; /* core */
  } /* namespace render */
} /* namespace vrt */


#endif // !defined __vrt_render_h_