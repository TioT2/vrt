#include "vrt.h"

namespace vrt::render::core
{
  VkBool32 kernel::DebugMessengerCallback
  (
    VkDebugUtilsMessageSeverityFlagBitsEXT      MessageSeverityFlag,
    VkDebugUtilsMessageTypeFlagsEXT             MessageTypeFlags,
    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    void*                                       UserData
  )
  {
    kernel *Kernel = reinterpret_cast<kernel *>(UserData);
    if (Kernel->InstanceSetupProcess)
      return VK_SUCCESS;
    const CHAR *MessageSeverityString;
    std::string MessageTypeString;

    switch (MessageSeverityFlag)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : MessageSeverityString = "VERBOSE";   break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    : MessageSeverityString = "INFO";      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : MessageSeverityString = "WARNING";   break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   : MessageSeverityString = "ERROR";     break;
    default                                              : MessageSeverityString = "<UNKNOWN>";
    }

    MessageTypeString = std::format
    (
      "{}{}{}{}",
      utils::CheckFlags(MessageTypeFlags, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT               ) ? "GENERAL "                : "",
      utils::CheckFlags(MessageTypeFlags, VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT            ) ? "VALIDATION "             : "",
      utils::CheckFlags(MessageTypeFlags, VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT           ) ? "PERFORMANCE "            : "",
      utils::CheckFlags(MessageTypeFlags, VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) ? "DEVICE_ADDRESS_BINDING " : ""
    );

    std::cerr << std::format(
      "--------------------------------\n"
      "Severity: {}\n"
      "Type: {}\n"
      "Message({}): {}\n",
      MessageSeverityString, MessageTypeString, CallbackData->pMessageIdName == nullptr ? "<NULLPTR>" : CallbackData->pMessageIdName, CallbackData->pMessage
    );

    return VK_SUCCESS;
  } /* kernel::DebugMessengerCallback */


  VkDebugUtilsMessengerCreateInfoEXT kernel::GetDebugMessengerCreateInfo( VOID )
  {
    return VkDebugUtilsMessengerCreateInfoEXT
    {
      /* VkStructureType                      */ .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      /* const void*                          */ .pNext = nullptr,
      /* VkDebugUtilsMessengerCreateFlagsEXT  */ .flags = 0,
      /* VkDebugUtilsMessageSeverityFlagsEXT  */ .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      /* VkDebugUtilsMessageTypeFlagsEXT      */ .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
      /* PFN_vkDebugUtilsMessengerCallbackEXT */ .pfnUserCallback = DebugMessengerCallback,
      /* void*                                */ .pUserData = this,
    };
  } /* GetDebugMessengerCreateInfo */
} /* namespace vrt::render */

/* vrt_debug.cpp */