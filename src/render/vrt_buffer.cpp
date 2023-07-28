#include "vrt.h"

namespace vrt::render
{
  buffer::buffer( core *Core ) : Core(Core)
  {

  } /* Core */

  buffer::buffer( const buffer &OtherConst )
  {
    *this = std::move(const_cast<buffer &>(OtherConst));
  } /* buffer */

  buffer & buffer::operator=( buffer &&Other ) noexcept
  {
    std::swap(Core, Other.Core);
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

    return vkGetBufferDeviceAddress(Core->Device, &DeviceAddressInfo);
  } /* GetDeviceAddress */

  VOID buffer::CopyTo( buffer &DestinationBuffer )
  {
    VkCommandBuffer CommandBuffer = Core->BeginTransfer();
    VkBufferCopy Copy { 0, 0, std::min(Size, DestinationBuffer.Size) };
    vkCmdCopyBuffer(CommandBuffer, Buffer, DestinationBuffer.Buffer, 1, &Copy);
    Core->EndTransfer(CommandBuffer);
  } /* CopyTo */


  VOID buffer::WriteData( VOID *Data, SIZE_T DataSize )
  {
    buffer StagingBuffer = Core->CreateBuffer(DataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    std::memcpy(StagingBuffer.MapMemory(), Data, DataSize);
    StagingBuffer.UnmapMemory();

    StagingBuffer.CopyTo(*this);
  } /* WriteData */


  VOID * buffer::MapMemory( VOID )
  {
    VOID *MemoryMap = nullptr;
    vkMapMemory(Core->Device, Memory, 0, Size, 0, &MemoryMap);
    return MemoryMap;
  } /* MapMemory */

  VOID buffer::UnmapMemory( VOID )
  {
    vkUnmapMemory(Core->Device, Memory);
  } /* UnmapMemory */

  buffer::~buffer( VOID )
  {
    if (Size != 0)
    {
      vkFreeMemory(Core->Device, Memory, nullptr);
      vkDestroyBuffer(Core->Device, Buffer, nullptr);
    }
  } /* ~buffer */
} /* namespace vrt::render */
