#ifndef __vrt_def_h_
#define __vrt_def_h_


#define _CRT_SECURE_NO_WARNINGS

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
    #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
    // Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
    // allocations to be of _CLIENT_BLOCK type
#else
    #define DBG_NEW new
#endif


#include <iostream>
#include <functional>
#include <format>
#include <span>
#include <chrono>
#include <optional>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <algorithm>
#include <filesystem>

#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 26819)
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_image.lib")

#pragma comment(lib, "volk.lib")
#pragma comment(lib, "dxcompiler.lib")

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <Volk/volk.h>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_image.h>
#include <SDL_test.h>
#pragma warning(pop)

namespace vrt
{
  using BYTE = unsigned char;
  using VOID = void;

  using INT = int;

  using SIZE_T = unsigned __int64;

  using UINT = unsigned int;
  using UINT32 = unsigned __int32;
  using UINT64 = unsigned __int64;

  using CHAR = char;

  using BOOL = bool;
  inline constexpr BOOL TRUE = true;
  inline constexpr BOOL FALSE = false;

  using FLOAT = float;
  using DOUBLE = double;

  using WORD = unsigned short;
  using DWORD = unsigned long;

  constexpr WORD operator""_word( const CHAR *str, SIZE_T Size )
  {
    return str[0] | ((WORD)str[1] << 8);
  }

  constexpr BOOL IS_DEBUG =
  #if defined _DEBUG
    TRUE
  #else
    FALSE
  #endif
  ;
} /* vrt */

#endif // !defined __vrt_def_h_

/* vrt_def.h */