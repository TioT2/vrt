#include "vrt.h"

namespace vrt::render::core
{
  image kernel::CreateImage( UINT32 Width, UINT32 Height, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlags MemoryProperties )
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


  VOID kernel::TransferImageLayout( image &Image, VkImageLayout OldLayout, VkImageLayout NewLayout )
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
  } /* TransferImageLayout */

  VkImageView kernel::CreateImageView( VkImage Image, VkFormat ImageFormat )
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

  image::image( kernel *Kernel ) : Kernel(Kernel)
  {

  } /* image */

  image::image( image &&Other )
  {
    *this = std::move(Other);
  } /* image */

  inline image & image::operator=( image &&Other ) noexcept
  {
    std::swap(Kernel, Other.Kernel);
    std::swap(Image, Other.Image);
    std::swap(Memory, Other.Memory);
    std::swap(Format, Other.Format);
    std::swap(ImageView, Other.ImageView);
    std::swap(MemoryTypeIndex, Other.MemoryTypeIndex);
    std::swap(Width, Other.Width);
    std::swap(Height, Other.Height);

    return *this;
  } /* operator= */

  VOID kernel::Destroy( image &Image )
  {
    vkDestroyImageView(Device, Image.ImageView, nullptr);
    vkDestroyImage(Device, Image.Image, nullptr);
    vkFreeMemory(Device, Image.Memory, nullptr);
  } /* Destroy */
} /* vrt::render */