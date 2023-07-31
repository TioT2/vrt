#include "vrt.h"

#include <windows.h>
#include <dxc/dxcapi.h>

namespace vrt::render::core
{
  VkShaderModule & rt_shader::GetModule( module_type Type )
  {
    switch(Type)
    {
    case rt_shader::module_type::RAYGEN       : return Raygen      ;
    case rt_shader::module_type::ANY_HIT      : return AnyHit      ;
    case rt_shader::module_type::CLOSEST_HIT  : return ClosestHit  ;
    case rt_shader::module_type::MISS         : return Miss        ;
    case rt_shader::module_type::INTERSECTION : return Intersection;
    default                                   : return Callable    ;
    }
  } /* GetModule */

  const CHAR * rt_shader::GetModuleTypeEntryPointName( module_type Type )
  {
    static const CHAR *EntryPointNames[] {"rrs_main", "ras_main", "rcs_main", "rms_main", "ris_main", "rfs_main"};

    return EntryPointNames[static_cast<SIZE_T>(Type)];
  } /* GetModuleEntryPointName */

  const CHAR * rt_shader::GetModuleTypeName( module_type Type )
  {
    static const CHAR *ModuleNames[] {"ray generation", "any hit", "closest hit", "miss", "any intersection", "callable"};

    return ModuleNames[static_cast<SIZE_T>(Type)];
  } /* GetModuleTypeName */


  rt_shader kernel::LoadRTShader( std::string_view Name )
  {
    shader_compiler Compiler;

    module_compilation_info Infos[]
    {
      {L"rrs_main", L"rrs", L"lib"},
      {L"ras_main", L"ras", L"lib"},
      {L"rcs_main", L"rcs", L"lib"},
      {L"rms_main", L"rms", L"lib"},
      {L"ris_main", L"ris", L"lib"},
      {L"rfs_main", L"rfs", L"lib"}
    };

    Compiler.Compile(Name, Infos, Device);

    return rt_shader
    {
      .Kernel       = this,
      .Raygen       = Infos[0].Module,
      .AnyHit       = Infos[1].Module,
      .ClosestHit   = Infos[2].Module,
      .Miss         = Infos[3].Module,
      .Intersection = Infos[4].Module,
      .Callable     = Infos[5].Module,
    };
  } /* LoadRTShader */

  std::vector<VkPipelineShaderStageCreateInfo> shader::GetPipelineShaderStageCreateInfos( VOID )
  {
    struct shader_stage_description
    {
      VkShaderStageFlagBits Stage;
      shader::module_type ModuleType;
    };
    shader_stage_description ShaderStageDescriptions[]
    {
      {VK_SHADER_STAGE_VERTEX_BIT,                  shader::module_type::VERTEX      },
      {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,    shader::module_type::TESS_CONTROL},
      {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, shader::module_type::TESS_EVAL   },
      {VK_SHADER_STAGE_GEOMETRY_BIT,                shader::module_type::GEOMETRY    },
      {VK_SHADER_STAGE_FRAGMENT_BIT,                shader::module_type::FRAGMENT    },
    };

    std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;

    for (const shader_stage_description &Descr : ShaderStageDescriptions)
    {
      VkShaderModule Module = GetModule(Descr.ModuleType);

      if (Module != VK_NULL_HANDLE)
        ShaderStageCreateInfos.push_back
        (
          VkPipelineShaderStageCreateInfo
          {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = Descr.Stage,
            .module = Module,
            .pName = shader::GetModuleTypeEntryPointName(Descr.ModuleType),
            .pSpecializationInfo = nullptr,
          }
        );
    }

    return ShaderStageCreateInfos;
  } /* GetPipelineShaderStageCreateInfos */

  VkShaderModule & shader::GetModule( module_type Type )
  {
    switch (Type)
    {
    case module_type::VERTEX       : return Vertex;
    case module_type::FRAGMENT     : return Fragment;
    case module_type::GEOMETRY     : return Geometry;
    case module_type::TESS_EVAL    : return TessEval;
    case module_type::TESS_CONTROL : return TessControl;

    default                        : return Compute;
    }
  } /* GetModule */

  const CHAR * shader::GetModuleTypeEntryPointName( module_type Type )
  {
    static const CHAR *EntryPoints[] {"vs_main", "hs_main", "ds_main", "gs_main", "ps_main", "cs_main"};

    return EntryPoints[static_cast<SIZE_T>(Type)];
  } /* GetModuleTypeEntryPointName */

  const CHAR * shader::GetModuleTypeName( module_type Type )
  {
    static const CHAR *EntryPoints[] {"vertex", "tesselation control", "tesselation evaluation", "geometry", "fragment", "compute"};

    return EntryPoints[static_cast<SIZE_T>(Type)];
  } /* GetModuleTypeName */

  shader kernel::LoadShader( std::string_view Name )
  {
    shader_compiler Compiler;

    module_compilation_info Infos[]
    {
      {L"vs_main", L"vs", L"vs"},
      {L"ps_main", L"ps", L"ps"},
      {L"gs_main", L"gs", L"gs"},
      {L"ds_main", L"ds", L"ds"},
      {L"hs_main", L"hs", L"hs"},
      {L"cs_main", L"cs", L"cs"}
    };

    Compiler.Compile(Name, Infos, Device);

    return shader
    {
      .Kernel       = this,
      .Vertex      = Infos[0].Module,
      .Fragment    = Infos[1].Module,
      .Geometry    = Infos[2].Module,
      .TessEval    = Infos[3].Module,
      .TessControl = Infos[4].Module,
      .Compute     = Infos[5].Module,
    };
  } /* LoadShader */

  /* rt shader destroying function */
  VOID kernel::Destroy( rt_shader &Shader )
  {
    if (Shader.Raygen       != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Raygen,       nullptr);
    if (Shader.AnyHit       != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.AnyHit,       nullptr);
    if (Shader.ClosestHit   != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.ClosestHit,   nullptr);
    if (Shader.Miss         != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Miss,         nullptr);
    if (Shader.Intersection != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Intersection, nullptr);
    if (Shader.Callable     != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Callable,     nullptr);
  } /* Destroy */

  /* rt shader destroying function */
  VOID kernel::Destroy( shader &Shader )
  {
    if (Shader.Vertex      != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Vertex,      nullptr);
    if (Shader.Fragment    != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Fragment,    nullptr);
    if (Shader.Geometry    != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Geometry,    nullptr);
    if (Shader.TessEval    != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.TessEval,    nullptr);
    if (Shader.TessControl != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.TessControl, nullptr);
    if (Shader.Compute     != VK_NULL_HANDLE) vkDestroyShaderModule(Device, Shader.Compute,     nullptr);
  } /* Destroy */
} /* vrt::render */