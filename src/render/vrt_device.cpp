#include "vrt.h"

namespace vrt::render::core
{
  UINT32 kernel::RatePhysicalDevice( VkPhysicalDevice PhysicalDevice )
  {
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures
    {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
      .pNext = nullptr,
    };
    VkPhysicalDeviceFeatures2 Features
    {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &RayTracingPipelineFeatures,
    };

    vkGetPhysicalDeviceFeatures2(PhysicalDevice, &Features);

    return Features.features.tessellationShader * Features.features.geometryShader * RayTracingPipelineFeatures.rayTracingPipeline;
  } /* RatePhysicalDevice */

  VOID kernel::ChoosePhysicalDevice( VOID )
  {
    UINT32 PhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> PhysicalDevices {PhysicalDeviceCount};
    vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());

    UINT32 BestPhysicalDeviceRate = 0;
    VkPhysicalDevice BestPhysicalDevice = VK_NULL_HANDLE;

    for (VkPhysicalDevice Device : PhysicalDevices)
    {
      UINT32 Rate = RatePhysicalDevice(Device);
      if (Rate > BestPhysicalDeviceRate)
      {
        BestPhysicalDeviceRate = Rate;
        BestPhysicalDevice = Device;
      }
    }

    PhysicalDevice = BestPhysicalDevice;

    utils::Assert(PhysicalDevice != VK_NULL_HANDLE, "no physical devices with VulkanAPI support");

    // get queue_family_indices
    UINT32 QueuefamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueuefamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties {QueuefamilyPropertyCount};
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueuefamilyPropertyCount, QueueFamilyProperties.data());

    // setup queue family properties
    for (UINT32 i = 0; i < QueuefamilyPropertyCount; i++)
    {
      if (utils::CheckFlags(QueueFamilyProperties[i].queueFlags, VK_QUEUE_GRAPHICS_BIT)) QueueFamilies.Graphics = i;
      if (utils::CheckFlags(QueueFamilyProperties[i].queueFlags, VK_QUEUE_TRANSFER_BIT)) QueueFamilies.Transfer = i;
      if (utils::CheckFlags(QueueFamilyProperties[i].queueFlags, VK_QUEUE_COMPUTE_BIT))  QueueFamilies.Compute  = i;

      VkBool32 SurfaceSupport = VK_FALSE;
      utils::AssertResult(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &SurfaceSupport));
      if (SurfaceSupport == VK_TRUE)
        QueueFamilies.Present = i;
    }

    std::set<UINT32> UniqueQueueFamilies {QueueFamilies.Compute, QueueFamilies.Graphics, QueueFamilies.Present, QueueFamilies.Transfer};
    QueueFamilies.Unique.reserve(UniqueQueueFamilies.size());
    for (UINT32 Family : UniqueQueueFamilies)
      QueueFamilies.Unique.push_back(Family);

    utils::Assert
    (
      QueueFamilies.Graphics != UINT32_MAX &&
      QueueFamilies.Present  != UINT32_MAX &&
      QueueFamilies.Transfer != UINT32_MAX &&
      QueueFamilies.Compute  != UINT32_MAX,
      "no required queue family presented"
    );
  } /* ChoosePhysicalDevice */

  VOID kernel::InitializeDevice( VOID )
  {
    ChoosePhysicalDevice();

    std::vector<VkDeviceQueueCreateInfo> DeviceQueueCreateInfos;
    DeviceQueueCreateInfos.reserve(QueueFamilies.Unique.size());

    FLOAT QueuePriority = 1.0f;
    for (UINT32 FamilyIndex : QueueFamilies.Unique)
      DeviceQueueCreateInfos.push_back
      (
        VkDeviceQueueCreateInfo
        {
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .queueFamilyIndex = FamilyIndex,
          .queueCount = 1,
          .pQueuePriorities = &QueuePriority,
        }
      );

    // enable all features
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR RTPipelineFeatures {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
    VkPhysicalDeviceRayQueryFeaturesKHR RayQueryFeatures {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR, &RTPipelineFeatures};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, &RayQueryFeatures};
    VkPhysicalDeviceBufferDeviceAddressFeatures BufferDeviceAddressFeatures {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, &AccelerationStructureFeatures};
    VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexingFeatures {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, &BufferDeviceAddressFeatures};
    VkPhysicalDeviceFeatures2 EnabledDeviceFeatures {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &DescriptorIndexingFeatures};

    vkGetPhysicalDeviceFeatures2(PhysicalDevice, &EnabledDeviceFeatures);
    vkGetPhysicalDeviceProperties2(PhysicalDevice, &PhysicalDeviceProperties.Properties2);

    VkDeviceCreateInfo CreateInfo
    {
      /* VkStructureType                 */ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      /* const void*                     */ .pNext = &EnabledDeviceFeatures,
      /* VkDeviceCreateFlags             */ .flags = 0,
      /* uint32_t                        */ .queueCreateInfoCount = static_cast<UINT32>(DeviceQueueCreateInfos.size()),
      /* const VkDeviceQueueCreateInfo*  */ .pQueueCreateInfos = DeviceQueueCreateInfos.data(),
      /* uint32_t                        */ .enabledLayerCount = (UINT32)EnabledDeviceLayers.size(),
      /* const char* const*              */ .ppEnabledLayerNames = EnabledDeviceLayers.data(),
      /* uint32_t                        */ .enabledExtensionCount = static_cast<UINT32>(EnabledDeviceExtensions.size()),
      /* const char* const*              */ .ppEnabledExtensionNames = EnabledDeviceExtensions.data(),
      /* const VkPhysicalDeviceFeatures* */ .pEnabledFeatures = nullptr,
    };

    utils::AssertResult(vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &Device), "error creating device");
    volkLoadDevice(Device);

    vkGetDeviceQueue(Device, QueueFamilies.Compute, 0, &ComputeQueue);
    vkGetDeviceQueue(Device, QueueFamilies.Graphics, 0, &GraphicsQueue);
    vkGetDeviceQueue(Device, QueueFamilies.Present, 0, &PresentQueue);
    vkGetDeviceQueue(Device, QueueFamilies.Transfer, 0, &TransferQueue);
  } /* InitPhysicalDevice */
} /* namespace vrt::render */