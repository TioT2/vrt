#include "vrt.h"

namespace vrt::render::core
{
  /* instance initialization function */
  VOID kernel::InitializeInstance( VOID )
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


    auto DebugMessengerCreateInfo = GetDebugMessengerCreateInfo();

    vkCreateDebugUtilsMessengerEXT(Instance, &DebugMessengerCreateInfo, nullptr, &DebugMessenger);
  } /* InitInstance */

  VOID kernel::InitializeCommandPools( VOID )
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
} /* namespace vrt::render::core */