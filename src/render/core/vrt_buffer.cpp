#include "vrt.h"

namespace vrt::render::core
{
  UINT32 kernel::FindMemoryType( UINT32 TypeFilter, VkMemoryPropertyFlags PropertyFlags )
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

  buffer kernel::CreateBuffer( VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemoryPropertyFlags )
  {
    buffer Buffer {this};

    Buffer.Size = Size;

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
      .allocationSize = MemoryRequirements.size,
      .memoryTypeIndex = Buffer.MemoryTypeIndex,
    };

    utils::AssertResult(vkAllocateMemory(Device, &AllocateInfo, nullptr, &Buffer.Memory), "error allocating memory");

    vkBindBufferMemory(Device, Buffer.Buffer, Buffer.Memory, 0);

    return std::move(Buffer);
  } /* CreateBuffer */

  buffer::buffer( kernel *Kernel ) : Kernel(Kernel)
  {

  } /* Kernel */

  buffer::buffer( const buffer &OtherConst )
  {
    *this = std::move(const_cast<buffer &>(OtherConst));
  } /* buffer */

  buffer & buffer::operator=( buffer &&Other ) noexcept
  {
    std::swap(Kernel, Other.Kernel);
    std::swap(Buffer, Other.Buffer);
    std::swap(Memory, Other.Memory);
    std::swap(Size, Other.Size);
    std::swap(MemoryTypeIndex, Other.MemoryTypeIndex);

    return *this;
  } /* buffer::operator= */

  VkDeviceAddress buffer::GetDeviceAddress( VOID ) const
  {
    VkBufferDeviceAddressInfo DeviceAddressInfo
    {
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .pNext = nullptr,
      .buffer = Buffer,
    };

    return vkGetBufferDeviceAddress(Kernel->Device, &DeviceAddressInfo);
  } /* GetDeviceAddress */

  VOID buffer::CopyTo( buffer &DestinationBuffer )
  {
    VkCommandBuffer CommandBuffer = Kernel->BeginTransfer();
    VkBufferCopy Copy { 0, 0, std::min(Size, DestinationBuffer.Size) };
    vkCmdCopyBuffer(CommandBuffer, Buffer, DestinationBuffer.Buffer, 1, &Copy);
    Kernel->EndTransfer(CommandBuffer);
  } /* CopyTo */


  VOID buffer::WriteData( const VOID *Data, SIZE_T DataSize )
  {
    buffer StagingBuffer = Kernel->CreateBuffer(DataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    std::memcpy(StagingBuffer.MapMemory(), Data, DataSize);
    StagingBuffer.UnmapMemory();

    StagingBuffer.CopyTo(*this);
    Kernel->Destroy(StagingBuffer);
  } /* WriteData */


  VOID * buffer::MapMemory( VOID )
  {
    VOID *MemoryMap = nullptr;
    vkMapMemory(Kernel->Device, Memory, 0, Size, 0, &MemoryMap);
    return MemoryMap;
  } /* MapMemory */

  VOID buffer::UnmapMemory( VOID )
  {
    vkUnmapMemory(Kernel->Device, Memory);
  } /* UnmapMemory */

  VOID kernel::Destroy( buffer &Buffer )
  {
    vkFreeMemory(Device, Buffer.Memory, nullptr);
    vkDestroyBuffer(Device, Buffer.Buffer, nullptr);
  } /* buffer */
} /* namespace vrt::render */
