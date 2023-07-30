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
    std::wstring WstringName {Name.begin(), Name.end()};
    std::wstring FolderPath = std::format(L"./{}/", WstringName.substr(0, WstringName.find_last_of('/')));

    HRESULT HRes;
    
    IDxcLibrary *Library;
    HRes = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));

    IDxcCompiler3 *Compiler;
    HRes = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
    
    IDxcUtils *Utils;
    HRes = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils));


    struct include_handler : IDxcIncludeHandler
    {
      IDxcUtils *Utils;

      include_handler( IDxcUtils *Utils ) : Utils(Utils)
      {

      } /* IncludeHandler */

      HRESULT STDMETHODCALLTYPE LoadSource( _In_z_ LPCWSTR FileName, _COM_Outptr_result_maybenull_ IDxcBlob **ppIncludeSource ) override
      {
        IDxcBlobEncoding *SourceBlob;
        UINT32 CodePage = DXC_CP_ACP;
        HRESULT HRes = Utils->LoadFile(FileName, &CodePage, &SourceBlob);

        if (ppIncludeSource != nullptr)
          *ppIncludeSource = SourceBlob;

        return HRes;
      } /* LoadSource */

      HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject ) override
      {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
      } /* QueryInterface */

      ULONG STDMETHODCALLTYPE AddRef( void ) override
      {
        return 0;
      } /* AddRef */

      ULONG STDMETHODCALLTYPE Release( void ) override
      {
        return 0;
      } /* Release */
    }; /* include_handler */
      
    include_handler IncludeHandler {Utils};
    rt_shader Shader;

    struct compilation_info
    {
      VkShaderModule &Module;
      rt_shader::module_type Type;
    } CompilationInfo[]
    {
      {Shader.Raygen,       rt_shader::module_type::RAYGEN,       },
      {Shader.AnyHit,       rt_shader::module_type::ANY_HIT,      },
      {Shader.ClosestHit,   rt_shader::module_type::CLOSEST_HIT,  },
      {Shader.Miss,         rt_shader::module_type::MISS,         },
      {Shader.Intersection, rt_shader::module_type::INTERSECTION, },
      {Shader.Callable,     rt_shader::module_type::CALLABLE,     }
    };

    std::wstring FileName = WstringName + L".hlsl";
    UINT32 CodePage = DXC_CP_ACP;
    IDxcBlobEncoding *SourceBlob;
    HRes = Utils->LoadFile(FileName.data(), &CodePage, &SourceBlob);

    if (FAILED(HRes))
      return Shader;

    DxcBuffer Buffer
    {
      .Ptr = SourceBlob->GetBufferPointer(),
      .Size = SourceBlob->GetBufferSize(),
      .Encoding = CodePage,
    };

    for (compilation_info &Info : CompilationInfo)
    {
      std::string_view CharEntryPointName = rt_shader::GetModuleTypeEntryPointName(Info.Type);
      std::wstring EntryPointName {CharEntryPointName.begin(), CharEntryPointName.end()};

      LPCWSTR CompilationArguments[]
      {
        FileName.data(),
        L"-E", EntryPointName.data(),
        L"-T", L"lib_6_4",
        L"-I", FolderPath.data(),
        L"-spirv",
        L"-fspv-target-env=vulkan1.3",
        L"-fspv-extension=SPV_KHR_ray_tracing",
        L"-fspv-extension=SPV_KHR_ray_query",
      };

      IDxcResult *Result = nullptr;
      HRes = Compiler->Compile(&Buffer, CompilationArguments, static_cast<UINT32>(std::size(CompilationArguments)), &IncludeHandler, IID_PPV_ARGS(&Result));

      if (SUCCEEDED(HRes))
        Result->GetStatus(&HRes);

      if (SUCCEEDED(HRes))
      {
        IDxcBlob *CodeBlob = nullptr;

        Result->GetResult(&CodeBlob);

        VkShaderModuleCreateInfo CreateInfo
        {
          /* VkStructureType           */ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
          /* const void*               */ .pNext = nullptr,
          /* VkShaderModuleCreateFlags */ .flags = 0,
          /* size_t                    */ .codeSize = static_cast<UINT32>(CodeBlob->GetBufferSize()),
          /* const uint32_t*           */ .pCode = static_cast<UINT32 *>(CodeBlob->GetBufferPointer()),
        };

        VkResult ModuleCreateResult = vkCreateShaderModule(Device, &CreateInfo, nullptr, &Info.Module);

        if (ModuleCreateResult != VK_SUCCESS)
        {
          std::cout << "Error creating module.\n";
          Info.Module = VK_NULL_HANDLE;
        }

        CodeBlob->Release();
      }
      else
      {
        IDxcBlobEncoding *ErrorBlob = nullptr;
        HRes = Result->GetErrorBuffer(&ErrorBlob);
        std::string_view Error = (const CHAR *)ErrorBlob->GetBufferPointer();

        if (Error != "error: missing entry point definition\n")
          std::cerr << std::format("Error compiling \"{}\" shader as {}: {}\n", std::string(FileName.begin(), FileName.end()), rt_shader::GetModuleTypeName(Info.Type), Error);
        Info.Module = VK_NULL_HANDLE;

        ErrorBlob->Release();
      }

      Result->Release();
    }

    SourceBlob->Release();

    Library->Release();
    Compiler->Release();
    Utils->Release();

    return Shader;
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
    std::wstring WstringName {Name.begin(), Name.end()};
    std::wstring FolderPath = std::format(L"./{}/", WstringName.substr(0, WstringName.find_last_of('/')));

    HRESULT HRes;
    
    IDxcLibrary *Library;
    HRes = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));

    IDxcCompiler3 *Compiler;
    HRes = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
    
    IDxcUtils *Utils;
    HRes = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils));


    struct include_handler : IDxcIncludeHandler
    {
      IDxcUtils *Utils;

      include_handler( IDxcUtils *Utils ) : Utils(Utils)
      {

      } /* IncludeHandler */

      HRESULT STDMETHODCALLTYPE LoadSource( _In_z_ LPCWSTR FileName, _COM_Outptr_result_maybenull_ IDxcBlob **ppIncludeSource ) override
      {
        IDxcBlobEncoding *SourceBlob;
        UINT32 CodePage = DXC_CP_ACP;
        HRESULT HRes = Utils->LoadFile(FileName, &CodePage, &SourceBlob);

        if (ppIncludeSource != nullptr)
          *ppIncludeSource = SourceBlob;

        return HRes;
      } /* LoadSource */

      HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject ) override
      {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
      } /* QueryInterface */

      ULONG STDMETHODCALLTYPE AddRef( void ) override
      {
        return 0;
      } /* AddRef */

      ULONG STDMETHODCALLTYPE Release( void ) override
      {
        return 0;
      } /* Release */
    }; /* include_handler */
      
    include_handler IncludeHandler {Utils};
    shader Shader;

    struct compilation_info
    {
      VkShaderModule &Module;
      shader::module_type Type;
      const WCHAR *EntryPointName;
      const WCHAR *DXPrefix;
    } CompilationInfo[]
    {
      {Shader.Vertex,      shader::module_type::VERTEX,       L"vs_main", L"vs"},
      {Shader.Fragment,    shader::module_type::FRAGMENT,     L"ps_main", L"ps"},
      {Shader.TessControl, shader::module_type::TESS_CONTROL, L"hs_main", L"hs"},
      {Shader.TessEval,    shader::module_type::TESS_EVAL,    L"ds_main", L"ds"},
      {Shader.Geometry,    shader::module_type::GEOMETRY,     L"gs_main", L"gs"},
      {Shader.Compute,     shader::module_type::COMPUTE,      L"cs_main", L"cs"}
    };

    if (std::filesystem::exists(std::format("{}.vs.hlsl", Name)) || std::filesystem::exists(std::format("{}.cs.hlsl", Name)))
    {
      for (compilation_info &Info : CompilationInfo)
      {
        std::wstring FileName = std::format(L"{}.{}.hlsl", WstringName, Info.DXPrefix);

        UINT32 CodePage = DXC_CP_ACP;
        IDxcBlobEncoding *SourceBlob;
        HRes = Utils->LoadFile(FileName.data(), &CodePage, &SourceBlob);

        if (FAILED(HRes))
          continue;

        DxcBuffer Buffer
        {
          .Ptr = SourceBlob->GetBufferPointer(),
          .Size = SourceBlob->GetBufferSize(),
          .Encoding = CodePage,
        }; 

        std::wstring ProfileName = std::format(L"{}_6_2", Info.DXPrefix);

        LPCWSTR CompilationArguments[]
        {
          FileName.data(),
          L"-E", Info.EntryPointName,
          L"-T", ProfileName.data(),
          L"-I", FolderPath.data(),
          L"-spirv",
        };
        IDxcResult *CompilationResult;
        HRes = Compiler->Compile(&Buffer, CompilationArguments, static_cast<UINT32>(std::size(CompilationArguments)), &IncludeHandler, IID_PPV_ARGS(&CompilationResult));

        if (SUCCEEDED(HRes))
          CompilationResult->GetStatus(&HRes);

        if (SUCCEEDED(HRes))
        {
          IDxcBlob *CodeBlob = nullptr;

          CompilationResult->GetResult(&CodeBlob);

          VkShaderModuleCreateInfo CreateInfo
          {
            /* VkStructureType           */ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            /* const void*               */ .pNext = nullptr,
            /* VkShaderModuleCreateFlags */ .flags = 0,
            /* size_t                    */ .codeSize = static_cast<UINT32>(CodeBlob->GetBufferSize()),
            /* const uint32_t*           */ .pCode = static_cast<UINT32 *>(CodeBlob->GetBufferPointer()),
          };

          VkResult ModuleCreateResult = vkCreateShaderModule(Device, &CreateInfo, nullptr, &Info.Module);

          if (ModuleCreateResult != VK_SUCCESS)
          {
            std::cout << "Error creating module.\n";
            Info.Module = VK_NULL_HANDLE;
          }

          CodeBlob->Release();
        }
        else
        {
          IDxcBlobEncoding *ErrorBlob = nullptr;
          HRes = CompilationResult->GetErrorBuffer(&ErrorBlob);
          std::string_view Error = (const CHAR *)ErrorBlob->GetBufferPointer();

          std::cerr << std::format("Error compiling \"{}\" shader: {}\n", std::string(FileName.begin(), FileName.end()), Error);
          Info.Module = VK_NULL_HANDLE;

          ErrorBlob->Release();
        }

        CompilationResult->Release();
        SourceBlob->Release();
      }
    }
    else
    {
      std::wstring FileName = WstringName + L".hlsl";
      UINT32 CodePage = DXC_CP_ACP;
      IDxcBlobEncoding *SourceBlob;
      HRes = Utils->LoadFile(FileName.data(), &CodePage, &SourceBlob);

      if (FAILED(HRes))
        return Shader;

      DxcBuffer Buffer
      {
        .Ptr = SourceBlob->GetBufferPointer(),
        .Size = SourceBlob->GetBufferSize(),
        .Encoding = CodePage,
      };

      for (compilation_info &Info : CompilationInfo)
      {
        std::wstring ProfileName = std::format(L"{}_6_2", Info.DXPrefix);

        LPCWSTR CompilationArguments[]
        {
          FileName.data(),
          L"-E", Info.EntryPointName,
          L"-T", ProfileName.data(),
          L"-I", FolderPath.data(),
          L"-spirv",
        };

        IDxcResult *Result = nullptr;
        HRes = Compiler->Compile(&Buffer, CompilationArguments, static_cast<UINT32>(std::size(CompilationArguments)), &IncludeHandler, IID_PPV_ARGS(&Result));

        if (SUCCEEDED(HRes))
          Result->GetStatus(&HRes);

        if (SUCCEEDED(HRes))
        {
          IDxcBlob *CodeBlob = nullptr;

          Result->GetResult(&CodeBlob);

          VkShaderModuleCreateInfo CreateInfo
          {
            /* VkStructureType           */ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            /* const void*               */ .pNext = nullptr,
            /* VkShaderModuleCreateFlags */ .flags = 0,
            /* size_t                    */ .codeSize = static_cast<UINT32>(CodeBlob->GetBufferSize()),
            /* const uint32_t*           */ .pCode = static_cast<UINT32 *>(CodeBlob->GetBufferPointer()),
          };

          VkResult ModuleCreateResult = vkCreateShaderModule(Device, &CreateInfo, nullptr, &Info.Module);

          if (ModuleCreateResult != VK_SUCCESS)
          {
            std::cout << "Error creating module.\n";
            Info.Module = VK_NULL_HANDLE;
          }

          CodeBlob->Release();
        }
        else
        {
          IDxcBlobEncoding *ErrorBlob = nullptr;
          HRes = Result->GetErrorBuffer(&ErrorBlob);
          std::string_view Error = (const CHAR *)ErrorBlob->GetBufferPointer();

          if (Error != "error: missing entry point definition\n")
            std::cerr << std::format("Error compiling \"{}\" shader as {}: {}\n", std::string(FileName.begin(), FileName.end()), shader::GetModuleTypeName(Info.Type), Error);
          Info.Module = VK_NULL_HANDLE;

          ErrorBlob->Release();
        }

        Result->Release();
      }

      SourceBlob->Release();
    }

    Library->Release();
    Compiler->Release();
    Utils->Release();

    return Shader;
  } /* LoadRTShader */

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