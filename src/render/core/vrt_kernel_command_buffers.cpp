#include "vrt.h"

namespace vrt::render::core
{
  /* command buffer creating function */
  VkCommandBuffer kernel::CreateCommandBuffer( VkCommandPool CommandPool )
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

  VkCommandBuffer kernel::BeginSingleTimeCommands( VOID )
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

  VOID kernel::EndSingleTimeCommands( VkCommandBuffer SingleTimeCommandBuffer )
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

  VkCommandBuffer kernel::BeginTransfer( VOID )
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

  VOID kernel::EndTransfer( VkCommandBuffer CommandBuffer )
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
} /* namespace vrt::render::core */