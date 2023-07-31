#include "vrt.h"

#include <windows.h>
#include <dxc/dxcapi.h>

namespace vrt::render::core
{
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

  VOID shader_compiler::Compile( std::string_view Path, std::span<module_compilation_info> CompilationInfos, VkDevice Device )
  {
    HRESULT HRes;
    
    IDxcLibrary *Library;
    HRes = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));

    IDxcCompiler3 *Compiler;
    HRes = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
    
    IDxcUtils *Utils;
    HRes = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils));

    include_handler IncludeHandler {Utils};

    // trying to load single file
    UINT32 CodePage = DXC_CP_ACP;


    std::wstring SingleFileName = std::format(L"{}.hlsl", std::wstring {Path.begin(), Path.end()});
    std::string_view CharFolderName = Path.substr(0, Path.find_last_of('/'));
    std::wstring FolderName = std::format(L"./{}/", std::wstring {CharFolderName.begin(), CharFolderName.end()});
    IDxcBlobEncoding *SingleFileBlob = nullptr;

    HRes = Utils->LoadFile(SingleFileName.data(), &CodePage, &SingleFileBlob);

    /* Shader is single file, all descriptions from */
    if (SUCCEEDED(HRes))
    {
      DxcBuffer SourceBuffer
      {
        .Ptr = SingleFileBlob->GetBufferPointer(),
        .Size = SingleFileBlob->GetBufferSize(),
        .Encoding = CodePage,
      };

      for (module_compilation_info &CompilationInfo : CompilationInfos)
      {
        std::wstring Format = std::format(L"{}_6_7", CompilationInfo.HLSLProfilePrefix);

        LPCWSTR CompilationArguments[]
        {
          SingleFileName.data(),
          L"-E", CompilationInfo.EntryPointName.data(),
          L"-T", Format.data(),
          L"-I", FolderName.data(),
          L"-spirv",
          L"-fspv-target-env=vulkan1.3",
          L"-fspv-extension=SPV_KHR_ray_tracing",
          L"-fspv-extension=SPV_KHR_ray_query",
        };

        IDxcResult *ModuleCompilationResult = nullptr;

        HRes = Compiler->Compile(&SourceBuffer, CompilationArguments, (UINT32)std::size(CompilationArguments), &IncludeHandler, IID_PPV_ARGS(&ModuleCompilationResult));

        if (SUCCEEDED(HRes))
          ModuleCompilationResult->GetStatus(&HRes);

        if (SUCCEEDED(HRes))
        {
          IDxcBlob *SPIRVBlob = nullptr;

          ModuleCompilationResult->GetResult(&SPIRVBlob);

          VkShaderModuleCreateInfo ShaderModuleCreateInfo
          {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = (UINT32)SPIRVBlob->GetBufferSize(),
            .pCode = static_cast<UINT32 *>(SPIRVBlob->GetBufferPointer()),
          };

          if (vkCreateShaderModule(Device, &ShaderModuleCreateInfo, nullptr, &CompilationInfo.Module) != VK_SUCCESS)
          {
            std::cerr << std::format
            (
              "Error compiling \"{}\" entry point of \"{}\" shader\n",
              std::string(CompilationInfo.EntryPointName.begin(), CompilationInfo.EntryPointName.end()),
              std::string(SingleFileName.begin(), SingleFileName.end())
            );
            CompilationInfo.Module = VK_NULL_HANDLE;
          }

          SPIRVBlob->Release();
        }
        else
        {
          IDxcBlobEncoding *ErrorBlob = nullptr;
          HRes = ModuleCompilationResult->GetErrorBuffer(&ErrorBlob);
          std::string_view Error = (const CHAR *)ErrorBlob->GetBufferPointer();

          if (Error != "error: missing entry point definition\n")
            std::cerr << std::format
            (
              "Error compiling \"{}\" entry point of \"{}\" shader: {}\n",
              std::string(CompilationInfo.EntryPointName.begin(), CompilationInfo.EntryPointName.end()),
              std::string(SingleFileName.begin(), SingleFileName.end()),
              Error
            );
          CompilationInfo.Module = nullptr;

          ErrorBlob->Release();
        }

        ModuleCompilationResult->Release();
      }

      SingleFileBlob->Release();
    }
    else
      for (module_compilation_info &CompilationInfo : CompilationInfos)
      {
        IDxcBlobEncoding *SourceBlob = nullptr;

        std::wstring FileName = std::format(L"{}.{}.hlsl", std::wstring {Path.begin(), Path.end()}, CompilationInfo.ShaderTypePrefix);
        HRes = Utils->LoadFile(FileName.data(), &CodePage, &SourceBlob);

        if (SUCCEEDED(HRes))
        {
          std::wstring Format = std::format(L"{}_6_7", CompilationInfo.HLSLProfilePrefix);

          LPCWSTR CompilationArguments[]
          {
            SingleFileName.data(),
            L"-E", CompilationInfo.EntryPointName.data(),
            L"-T", L"lib_6_7",
            L"-I", FolderName.data(),
            L"-spirv",
            L"-fspv-target-env=vulkan1.3",
            L"-fspv-extension=SPV_KHR_ray_tracing",
            L"-fspv-extension=SPV_KHR_ray_query",
          };

          DxcBuffer SourceBuffer
          {
            .Ptr = SourceBlob->GetBufferPointer(),
            .Size = SourceBlob->GetBufferSize(),
            .Encoding = CodePage,
          };

          IDxcResult *ModuleCompilationResult = nullptr;

          HRes = Compiler->Compile(&SourceBuffer, CompilationArguments, std::size(CompilationArguments), &IncludeHandler, IID_PPV_ARGS(&ModuleCompilationResult));

          if (SUCCEEDED(HRes))
            ModuleCompilationResult->GetStatus(&HRes);

          if (SUCCEEDED(HRes))
          {
            IDxcBlob *SPIRVBlob = nullptr;

            ModuleCompilationResult->GetResult(&SPIRVBlob);

            VkShaderModuleCreateInfo ShaderModuleCreateInfo
            {
              .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
              .pNext = nullptr,
              .flags = 0,
              .codeSize = (UINT32)SPIRVBlob->GetBufferSize(),
              .pCode = static_cast<UINT32 *>(SPIRVBlob->GetBufferPointer()),
            };

            if (vkCreateShaderModule(Device, &ShaderModuleCreateInfo, nullptr, &CompilationInfo.Module) != VK_SUCCESS)
            {
              std::cerr << std::format("Error compiling \"{}\" shader\n", std::string(FileName.begin(), FileName.end()));
              CompilationInfo.Module = VK_NULL_HANDLE;
            }

            SPIRVBlob->Release();
          }
          else
          {
            IDxcBlobEncoding *ErrorBlob = nullptr;
            HRes = ModuleCompilationResult->GetErrorBuffer(&ErrorBlob);
            std::string_view Error = (const CHAR *)ErrorBlob->GetBufferPointer();

            if (Error != "error: missing entry point definition\n")
              std::cerr << std::format("Error compiling \"{}\" shader: {}\n", std::string(FileName.begin(), FileName.end()), Error);
            CompilationInfo.Module = nullptr;

            ErrorBlob->Release();
          }

          SourceBlob->Release();
        }
      }

    Library->Release();
    Compiler->Release();
    Utils->Release();
  } /* Compile */
} /* namespace vrt::render::core */