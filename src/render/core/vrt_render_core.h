#ifndef __vrt_render_core_h_
#define __vrt_render_core_h_

#include "vrt_mth.h"
#include "vrt_utils.h"
#include "vrt_memory.h"

namespace vrt
{
  namespace render
  {
    namespace core
    {
      class kernel;

      struct buffer
      {
        kernel *Kernel = nullptr;

        VkBuffer Buffer = VK_NULL_HANDLE;
        VkDeviceMemory Memory = VK_NULL_HANDLE;
        SIZE_T Size = 0;
        UINT32 MemoryTypeIndex = 0;

        /* constructor */
        buffer( kernel *Kernel = nullptr );

        /* !!!ACHTUNG!!! */
        buffer( const buffer &OtherConst );

        /* buffer move assignment */
        buffer & operator=( buffer &&Other ) noexcept;

        /* Get device address of this buffer. */
        VkDeviceAddress GetDeviceAddress( VOID ) const;

        /* copy buffer to destination */
        VOID CopyTo( buffer &DestinationBuffer );

        /* Write data to buffer using transfer buffer */
        VOID WriteData( const VOID *Data, SIZE_T DataSize );

        /* Map buffer memory to RAM address */
        VOID * MapMemory( VOID );

        /* Unmap buffer memory */
        VOID UnmapMemory( VOID );
      }; /* buffer */

      struct image
      {
        kernel *Kernel = nullptr;

        VkImage Image;
        VkDeviceMemory Memory;
        VkFormat Format;
        VkImageView ImageView;
        UINT32 MemoryTypeIndex;
        UINT32 Width, Height;

        image( kernel *Kernel = nullptr );

        image( image &&Other );

        inline image & operator=( image &&Other ) noexcept;
      }; /* image */

      struct rt_shader
      {
        enum struct module_type
        {
          RAYGEN       = 0,
          ANY_HIT      = 1,
          CLOSEST_HIT  = 2,
          MISS         = 3,
          INTERSECTION = 4,
          CALLABLE     = 5,
        };
        kernel *Kernel = nullptr;

        VkShaderModule Raygen       = VK_NULL_HANDLE;
        VkShaderModule AnyHit       = VK_NULL_HANDLE;
        VkShaderModule ClosestHit   = VK_NULL_HANDLE;
        VkShaderModule Miss         = VK_NULL_HANDLE;
        VkShaderModule Intersection = VK_NULL_HANDLE;
        VkShaderModule Callable     = VK_NULL_HANDLE;

        VkShaderModule & GetModule( module_type Type );

        static const CHAR * GetModuleTypeEntryPointName( module_type Type );

        static const CHAR * GetModuleTypeName( module_type Type );
      }; /* rt_shader */

      struct module_compilation_info
      {
        std::wstring_view EntryPointName;
        std::wstring_view ShaderTypePrefix;
        std::wstring_view HLSLProfilePrefix;
        VkShaderModule Module;
      };

      class shader_compiler
      {
      public:
        VOID Compile( std::string_view Path, std::span<module_compilation_info> CompilationDescription, VkDevice Device );
      }; /* shader_compiler */

      struct shader
      {
        enum struct module_type
        {
          VERTEX       = 0,
          TESS_CONTROL = 1,
          TESS_EVAL    = 2,
          GEOMETRY     = 3,
          FRAGMENT     = 4,
          COMPUTE      = 5,
        };
        constexpr static SIZE_T ModuleCount = 6;

        kernel *Kernel = nullptr;

        VkShaderModule Vertex      = VK_NULL_HANDLE;
        VkShaderModule Fragment    = VK_NULL_HANDLE;
        VkShaderModule Geometry    = VK_NULL_HANDLE;
        VkShaderModule TessEval    = VK_NULL_HANDLE;
        VkShaderModule TessControl = VK_NULL_HANDLE;
        VkShaderModule Compute     = VK_NULL_HANDLE;

        VkShaderModule & GetModule( module_type Type );

        static const CHAR * GetModuleTypeEntryPointName( module_type Type );

        static const CHAR * GetModuleTypeName( module_type Type );

        std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos( VOID );
      }; /* shader */

      struct material : resource<std::string>
      {
        kernel *Kernel = nullptr;

        VkShaderModule ClosestHitShader = VK_NULL_HANDLE; // closesthit shader

        ~material( VOID );
      }; /* material */


      /* Point light description structure */
      struct point_light
      {
        vec3 Position;  // Light position
        FLOAT Aligner0; // Aligner#0
        vec3 Color;     // Light color
        FLOAT Aligner1; // Aligner#1
      }; /* point_light */


      struct primitive : resource<UINT32>
      {
        kernel *Kernel = nullptr;
        material *Material = nullptr;

        SIZE_T VertexPositionComponentOffset = 0;
        SIZE_T VertexSize = 0;
        buffer VertexBuffer {};
        SIZE_T VertexCount = 0;
        buffer IndexBuffer {};
        SIZE_T IndexCount = 0;
        mat4 TrasnformMatrix = mat4::Identity();

        ~primitive( VOID ) override;
      }; /* primitive */

      class model : public resource<UINT32>
      {
      public:
        mat4 TransformMatrix = mat4::Identity();

        kernel *Kernel = nullptr;

        std::vector<primitive *> Primitives {};

        buffer BLASStorageBuffer {};
        VkAccelerationStructureKHR BLAS = VK_NULL_HANDLE;

        ~model( VOID );
      }; /* model */

      struct scene : resource<std::string>
      {
        kernel *Kernel = nullptr;

        camera Camera;
        buffer GlobalBuffer;

        VkPipeline Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;

        std::vector<point_light> Lights;
        buffer LightStorageBuffer;


        std::vector<model *> Models;
        buffer TLASStorageBuffer {};
        VkAccelerationStructureKHR TLAS = VK_NULL_HANDLE;
        buffer InstanceBuffer {};
        SIZE_T InstanceCount = 0;

        SIZE_T SBTAlignedGroupSize = 0;
        buffer SBTStorageBuffer; // Shader binding table storage buffer
        SIZE_T HitShaderGroupCount = 0;

        /* Scene destructor */
        ~scene( VOID );
      }; /* scene */


      struct small_vertex
      {
        vec3 Position;
        vec2 TexCoord;
        vec3 Normal;
      }; /* vertex */

      struct present_buffer_data
      {
        UINT32 CollectionFrameCount;
      }; /* present_buffer_data */

      struct topology
      {
        std::vector<small_vertex> Vertices;
        std::vector<UINT32> Indices;

        VOID CalculateNormals( VOID )
        {
          for (SIZE_T i = 0, len = Vertices.size(); i < len; i++)
            Vertices[i].Normal = vec3(0);

          if (Indices.empty())
          {
            for (SIZE_T i = 0, len = Vertices.size(); i < len; i += 3)
            {
              small_vertex
                &V0 = Vertices[i + 0],
                &V1 = Vertices[i + 1],
                &V2 = Vertices[i + 2];

              vec3 Normal = ((V1.Position - V0.Position) % (V2.Position - V0.Position)).Normalize();

              V0.Normal += Normal;
              V1.Normal += Normal;
              V2.Normal += Normal;
            }
          }
          else
          {
            for (SIZE_T i = 0, len = Indices.size(); i < len; i += 3)
            {
              small_vertex
                &V0 = Vertices[Indices[i + 0]],
                &V1 = Vertices[Indices[i + 1]],
                &V2 = Vertices[Indices[i + 2]];

              vec3 Normal = ((V1.Position - V0.Position) % (V2.Position - V0.Position)).Normalize();

              V0.Normal += Normal;
              V1.Normal += Normal;
              V2.Normal += Normal;
            }
          }

          for (SIZE_T i = 0, len = Vertices.size(); i < len; i++)
            Vertices[i].Normal.Normalize();
        } /* CalculateNormals */

        static topology LoadOBJ( std::string_view Path )
        {
          std::FILE * File = std::fopen(Path.data(), "r");

          if (File == nullptr)
            return {};

          topology Tpl;

          std::vector<vec3> Positions;
          std::vector<vec2> TexCoords;
          std::vector<vec3> Normals;

          CHAR Buffer[256];

          /* Optimization */
          std::map<std::tuple<SIZE_T, SIZE_T, SIZE_T>, SIZE_T> VertexMap;

          while (std::fgets(Buffer, static_cast<INT>(std::size(Buffer)), File))
          {
            utils::splitter Splitter {Buffer, ' '};
            utils::splitter FacetSplitter;

            std::string_view Head = Splitter.Get();

            CHAR *End = nullptr;

            switch (*(WORD *)Head.data())
            {
            case "v "_word:
              Positions.push_back(vec3());

              Positions.back().X = std::strtof(Splitter.Get().data(), &End);
              Positions.back().Y = std::strtof(Splitter.Get().data(), &End);
              Positions.back().Z = std::strtof(Splitter.Get().data(), &End);
              break;

            case "vt"_word:
              TexCoords.push_back(vec2());

              TexCoords.back().X = std::strtof(Splitter.Get().data(), &End);
              TexCoords.back().Y = std::strtof(Splitter.Get().data(), &End);
              break;

            case "vn"_word:
              Normals.push_back(vec3());

              Normals.back().X = std::strtof(Splitter.Get().data(), &End);
              Normals.back().Y = std::strtof(Splitter.Get().data(), &End);
              Normals.back().Z = std::strtof(Splitter.Get().data(), &End);
              break;

            case "f "_word:
              for (INT i = 0; i < 3; i++)
              {
                FacetSplitter = utils::splitter(Splitter.Get(), '/');
                SIZE_T PositionIndex = static_cast<SIZE_T>(std::strtol(FacetSplitter.Get().data(), &End, 10));
                SIZE_T TexCoordIndex = static_cast<SIZE_T>(std::strtol(FacetSplitter.Get().data(), &End, 10));
                SIZE_T NormalIndex   = static_cast<SIZE_T>(std::strtol(FacetSplitter.Get().data(), &End, 10));

                auto tuple = std::make_tuple(PositionIndex, TexCoordIndex, NormalIndex);

                auto iter = VertexMap.find(tuple);

                // Add vertex if it isn't yet.
                if (iter == VertexMap.end())
                {
                  iter = VertexMap.insert({tuple, Tpl.Vertices.size()}).first;

                  Tpl.Vertices.push_back(small_vertex {});
                  Tpl.Vertices.back().Position = (PositionIndex == 0 ? vec3(0) : Positions[PositionIndex - 1]);
                  Tpl.Vertices.back().TexCoord = (TexCoordIndex == 0 ? vec2(0) : TexCoords[TexCoordIndex - 1]);
                  Tpl.Vertices.back().Normal   = (NormalIndex   == 0 ? vec3(0) : Normals  [NormalIndex   - 1]);
                }

                Tpl.Indices.push_back(static_cast<UINT32>(iter->second));
              }
              break;
            }
          }

          std::fclose(File);

          return Tpl;
        } /* LoadOBJ */
      }; /* topology */

      struct global_buffer_data
      {
        vec3 CameraLocation;  // location of camera
        UINT32 LightNumber;   // number of pointlight's in scene
        vec3 CameraDirection; // direction of camera
        BOOL IsMoved;         // is camera moved on this frame
        vec3 CameraRight;     // camera right direction
        UINT32 FrameIndex;    // index of current frame (for random)
        vec3 CameraUp;        // camera up direction

        FLOAT _Aligner = 0;
        vec3 WidthHeightNear; // With, Height, Near planes
      }; /* global_buffer_data */

      class kernel : manager<scene, std::string>, manager<material, std::string>, manager<model>, manager<primitive>
      {
      public:
        SIZE_T CurrentFrame = 0;           // Current frame index

        SDL_Window *Window = nullptr;      // Window pointer
        BOOL InstanceSetupProcess = FALSE; // flag for validation layers not sh*tting in log by infinite DLL's loading und other 'useful log info'

        VkInstance Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;

        VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;

        struct queue_family_indices
        {
          UINT32
            Graphics = UINT32_MAX,
            Present  = UINT32_MAX,
            Transfer = UINT32_MAX,
            Compute  = UINT32_MAX;

          std::vector<UINT32> Unique;
        } QueueFamilies;

        VkDevice Device = VK_NULL_HANDLE;
        VkSurfaceKHR Surface = VK_NULL_HANDLE;

        VkQueue GraphicsQueue = VK_NULL_HANDLE;
        VkQueue PresentQueue = VK_NULL_HANDLE;
        VkQueue TransferQueue = VK_NULL_HANDLE;
        VkQueue ComputeQueue = VK_NULL_HANDLE;

        VkCommandPool GraphicsCommandPool = VK_NULL_HANDLE;
        VkCommandPool TransferCommandPool = VK_NULL_HANDLE;
        VkCommandPool ComputeCommandPool = VK_NULL_HANDLE;

        VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
        VkExtent2D SwapchainImageExtent;
        VkFormat SwapchainImageFormat;
        std::vector<VkImage> SwapchainImages;
        std::vector<VkImageView> SwapchainImageViews;
        std::vector<VkFramebuffer> Framebuffers;

        struct
        {
          VkPhysicalDeviceRayTracingPipelinePropertiesKHR RTPipelineProperties {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
          VkPhysicalDeviceAccelerationStructurePropertiesKHR AccelerationStructureProperties {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR, &RTPipelineProperties};
          VkPhysicalDeviceProperties2 Properties2 {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &AccelerationStructureProperties};
          VkPhysicalDeviceProperties &Properties = Properties2.properties;
        } PhysicalDeviceProperties;

        std::vector<const CHAR *> EnabledInstanceLayers { "VK_LAYER_KHRONOS_validation", };
        std::vector<const CHAR *> EnabledInstanceExtensions { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, };

        std::vector<const CHAR *> EnabledDeviceExtensions
        {
          VK_KHR_SWAPCHAIN_EXTENSION_NAME,
          VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
          VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
          VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
          VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
          VK_KHR_RAY_QUERY_EXTENSION_NAME,
        };
        std::vector<const CHAR *> EnabledDeviceLayers {};

        /* VulkanAPI provided Debug messenger callback function. */
        static VkBool32 DebugMessengerCallback(
          VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverityFlag, VkDebugUtilsMessageTypeFlagsEXT MessageTypeFlags,
          const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData );

        /* Debug messenger create info fitting for current instance getting function */
        VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo( VOID );

        /* instance initialization function */
        VOID InitializeInstance( VOID );

        /* Physical device raiting function */
        UINT32 RatePhysicalDevice( VkPhysicalDevice PhysicalDevice );

        /* Physical device selecting function */
        VOID ChoosePhysicalDevice( VOID );

        /* Device initialization function */
        VOID InitializeDevice( VOID );

        /* Swapchain initialization function */
        VOID InitializeSwapchain( VOID );

        /* Presentation renderpass initialization function */
        VOID InitializePresentRenderPass( VOID );

        /* Framebuffers initialization function */
        VOID InitializeFramebuffers( VOID );

        /* Initialize rendertarget */
        VOID InitializeTarget( VOID );

        /* Presentation pipeline initialization function */
        VOID InitializePresentPipeline( VOID );

        /* Presentation resources initialization function */
        VOID InitializePresentResources( VOID );

        /* CommandPools initialization */
        VOID InitializeCommandPools( VOID );

        /* image creating function */
        image CreateImage( UINT32 Width, UINT32 Height, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlags MemoryProperties );

        /* image layout transfering function */
        VOID TransferImageLayout( image &Image, VkImageLayout OldLayout, VkImageLayout NewLayout );

        /* image view creating function */
        VkImageView CreateImageView( VkImage Image, VkFormat ImageFormat );

        /* command buffer creating function */
        VkCommandBuffer CreateCommandBuffer( VkCommandPool CommandPool );

        /* Memory type choosing function */
        UINT32 FindMemoryType( UINT32 TypeFilter, VkMemoryPropertyFlags PropertyFlags );

        /* buffer creating function */
        buffer CreateBuffer( VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemoryPropertyFlags );

        VkCommandBuffer BeginSingleTimeCommands( VOID );

        VOID EndSingleTimeCommands( VkCommandBuffer SingleTimeCommandBuffer );

        VkCommandBuffer BeginTransfer( VOID );

        VOID EndTransfer( VkCommandBuffer CommandBuffer );

        /* raytracing shader loading function */
        rt_shader LoadRTShader( std::string_view Name );

        /* shader loading function */
        shader LoadShader( std::string_view Name );

        /* rt shader destroying function */
        VOID Destroy( rt_shader &Shader );

        /* shader destroying function */
        VOID Destroy( shader &Shader );

        /* Image destroying function */
        VOID Destroy( image &Image );

        /* Buffer destroying function */
        VOID Destroy( buffer &Buffer );

        /* Resize function */
        VOID Resize( VOID );

        /* ... */

        image TargetImage;
        buffer PresentBuffer;

        BOOL DoPresentCollection = FALSE;

        VkSampler TargetImageSampler = VK_NULL_HANDLE;
        VkRenderPass PresentRenderPass = VK_NULL_HANDLE;
        VkDescriptorSetLayout PresentDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet PresentDescriptorSet = VK_NULL_HANDLE;
        VkDescriptorPool PresentDescriptorPool = VK_NULL_HANDLE;
        VkPipelineLayout PresentPipelineLayout = VK_NULL_HANDLE;
        VkPipeline PresentPipeline = VK_NULL_HANDLE;
        VkCommandBuffer GraphicsCommandBuffer = VK_NULL_HANDLE;
        VkFence InFlightFence = VK_NULL_HANDLE;
        VkSemaphore ImageAvailableSemaphore = VK_NULL_HANDLE, RenderFinishedSemaphore = VK_NULL_HANDLE;

        scene *Scene = nullptr;

        material * CreateMaterial( const std::string &MaterialName, std::string_view ShaderName )
        {
          material *Mtl = manager<material, std::string>::CreateResource(MaterialName);

          module_compilation_info Info {L"rcs_main", L"rcs", L"lib"};
          shader_compiler Compiler;
          Compiler.Compile(ShaderName, std::span<module_compilation_info>(&Info, 1), Device);

          Mtl->Kernel = this;
          Mtl->ClosestHitShader = Info.Module;

          return Mtl;
        } /* CreateMaterial */

        template <typename vertex_type>
          primitive * CreatePrimitive( material *Material, std::span<const vertex_type> Vertices, SIZE_T PositionComponentOffset, std::span<const UINT32> Indices, const mat4 &TransformMatrix = mat4::Identity() )
          {
            primitive *Primitive = manager<primitive>::CreateResource();

            Primitive->Kernel = this;
            Primitive->Material = Material;

            Material->Grab();

            Primitive->TrasnformMatrix = TransformMatrix;
            /* initialize buffers */
            Primitive->VertexBuffer = CreateBuffer(Vertices.size_bytes(),
              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            Primitive->VertexBuffer.WriteData(Vertices.data(), Vertices.size_bytes());
            Primitive->VertexCount = Vertices.size();

            if (Indices.size() != 0)
            {
              Primitive->IndexBuffer = CreateBuffer(Indices.size_bytes(),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
              Primitive->IndexBuffer.WriteData(Indices.data(), Indices.size_bytes());
              Primitive->IndexCount = Indices.size();
            }

            Primitive->VertexSize = sizeof(vertex_type);
            Primitive->VertexPositionComponentOffset = PositionComponentOffset;

            return Primitive;
          } /* CreatePrimitive */

        /* @brief Model creating function.
         * @param[in] std::span<primitive *> Primitives - array of that this model include
         * @param[in] mat4 TransformMatrix - Model transformation matrix
         */
        model * CreateModel( std::span<primitive *> Primitives, mat4 TransformMatrix = mat4::Identity() )
        {
          model *Model = manager<model>::CreateResource();

          for (primitive *Prim : Primitives)
            Prim->Grab();
          Model->Primitives = {Primitives.begin(), Primitives.end()};

          Model->Kernel = this;
            
          Model->TransformMatrix = TransformMatrix;
          /* initialize buffers */

          std::vector<VkAccelerationStructureGeometryKHR> Geometries {Primitives.size()};

          for (UINT32 i = 0; i < Geometries.size(); i++)
          {
            Geometries[i] = VkAccelerationStructureGeometryKHR 
            {
              /* VkStructureType                        */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
              /* const void*                            */ .pNext = nullptr,
              /* VkGeometryTypeKHR                      */ .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
              /* VkAccelerationStructureGeometryDataKHR */ .geometry = {},
              /* VkGeometryFlagsKHR                     */ .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
            };

            VkAccelerationStructureGeometryTrianglesDataKHR &TrianglesData = Geometries[i].geometry.triangles;
            TrianglesData = VkAccelerationStructureGeometryTrianglesDataKHR
            {
              /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
              /* const void*                   */ .pNext = nullptr,
              /* VkFormat                      */ .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
              /* VkDeviceOrHostAddressConstKHR */ .vertexData = { .deviceAddress = Primitives[i]->VertexBuffer.GetDeviceAddress() + Primitives[i]->VertexPositionComponentOffset },
              /* VkDeviceSize                  */ .vertexStride = Primitives[i]->VertexSize,
              /* uint32_t                      */ .maxVertex = (UINT32)Primitives[i]->VertexCount,
              /* VkIndexType                   */ // .indexType = VK_INDEX_TYPE_UINT32,
              /* VkDeviceOrHostAddressConstKHR */ // .indexData = Primitive->IndexCount != 0 ? { .deviceAddress = Primitive->IndexBuffer.GetDeviceAddress() } : {},
              /* VkDeviceOrHostAddressConstKHR */ .transformData {},
            };

            if (Primitives[i]->IndexCount != 0)
            {
              TrianglesData.indexType = VK_INDEX_TYPE_UINT32;
              TrianglesData.indexData = { .deviceAddress = Primitives[i]->IndexBuffer.GetDeviceAddress() };
            }
            else
            {
              TrianglesData.indexType = VK_INDEX_TYPE_NONE_KHR;
              TrianglesData.indexData = {};
            }
          }

          VkAccelerationStructureBuildGeometryInfoKHR GeometryBuildInfo
          {
            /* VkStructureType                                  */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
            /* const void*                                      */ .pNext = nullptr,
            /* VkAccelerationStructureTypeKHR                   */ .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            /* VkBuildAccelerationStructureFlagsKHR             */ .flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
            /* VkBuildAccelerationStructureModeKHR              */ .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
            /* VkAccelerationStructureKHR                       */ .srcAccelerationStructure = VK_NULL_HANDLE,
            /* VkAccelerationStructureKHR                       */ .dstAccelerationStructure = VK_NULL_HANDLE,
            /* uint32_t                                         */ .geometryCount = (UINT32)Geometries.size(),
            /* const VkAccelerationStructureGeometryKHR*        */ .pGeometries = Geometries.data(),
            /* const VkAccelerationStructureGeometryKHR* const* */ .ppGeometries = nullptr,
            /* VkDeviceOrHostAddressKHR                         */ .scratchData = 0,
          };

          std::vector<UINT32> GeometryPrimitiveCounts;

          GeometryPrimitiveCounts.resize(Primitives.size());
          for (UINT32 i = 0; i < GeometryPrimitiveCounts.size(); i++)
            GeometryPrimitiveCounts[i] = (UINT32)std::max<SIZE_T>(Primitives[i]->VertexCount, Primitives[i]->IndexCount) / 3;

          VkAccelerationStructureBuildSizesInfoKHR BuildSizesInfo {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};

          vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &GeometryBuildInfo, GeometryPrimitiveCounts.data(), &BuildSizesInfo);
          Model->BLASStorageBuffer = CreateBuffer(BuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

          VkAccelerationStructureCreateInfoKHR AccelerationStructureCreateInfo
          {
            /* VkStructureType                       */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
            /* const void*                           */ .pNext = nullptr,
            /* VkAccelerationStructureCreateFlagsKHR */ .createFlags = 0,
            /* VkBuffer                              */ .buffer = Model->BLASStorageBuffer.Buffer,
            /* VkDeviceSize                          */ .offset = 0,
            /* VkDeviceSize                          */ .size = BuildSizesInfo.accelerationStructureSize,
            /* VkAccelerationStructureTypeKHR        */ .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            /* VkDeviceAddress                       */ .deviceAddress = 0,
          };

          std::vector<VkAccelerationStructureBuildRangeInfoKHR> BuildRangeInfos {Primitives.size()};

          for (UINT32 i = 0; i < BuildRangeInfos.size(); i++)
            BuildRangeInfos[i] = VkAccelerationStructureBuildRangeInfoKHR
            {
              .primitiveCount = GeometryPrimitiveCounts[i],
              .primitiveOffset = 0,
              .firstVertex = 0,
              .transformOffset = 0,
            };

          vkCreateAccelerationStructureKHR(Device, &AccelerationStructureCreateInfo, nullptr, &Model->BLAS);

          GeometryBuildInfo.dstAccelerationStructure = Model->BLAS;
          buffer ScratchBuffer = CreateBuffer(BuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
          GeometryBuildInfo.scratchData.deviceAddress = ScratchBuffer.GetDeviceAddress();

          // building acceleration buffer
          VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();
          VkAccelerationStructureBuildRangeInfoKHR *PtrRangeInfo = BuildRangeInfos.data();
          vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &GeometryBuildInfo, &PtrRangeInfo);
          EndSingleTimeCommands(CommandBuffer);

          Destroy(ScratchBuffer);

          return Model;
        } /* CreateModel */

        /* Scene building function */
        scene * CreateScene( const std::string &SceneName, std::span<model *> Models )
        {
          scene *Scene = manager<scene, std::string>::CreateResource(SceneName);

          for (model *Model : Models)
            Model->Grab();
          Scene->Models = {Models.begin(), Models.end()};

          Scene->Kernel = this;
          Scene->GlobalBuffer = CreateBuffer(sizeof(global_buffer_data), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


          // Add first light
          Scene->Lights =
          {
            {
              .Position = vec3(16, 16, 16),
              .Color    = vec3(50, 50, 50),
            },
            {
              .Position = vec3(-16, 16, 16),
              .Color    = vec3(100, 50, 0),
            },
            {
              .Position = vec3(16, 16, -16),
              .Color    = vec3(000, 50, 100),
            },
            // {
            //   .Position = vec3(0, 15, 0),
            //   .Color = vec3(0, 200, 0)
            // }
          };
          Scene->LightStorageBuffer = CreateBuffer(sizeof(point_light) * Scene->Lights.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
          Scene->LightStorageBuffer.WriteData(Scene->Lights.data(), sizeof(point_light) * Scene->Lights.size());


          Scene->InstanceCount = Models.size();

          // initialize 'vertex buffer' of TLAS
          Scene->InstanceBuffer = CreateBuffer
          (
            sizeof(VkAccelerationStructureInstanceKHR) * Models.size(),
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
          );

          VkAccelerationStructureInstanceKHR *InstanceData = reinterpret_cast<VkAccelerationStructureInstanceKHR *>(Scene->InstanceBuffer.MapMemory());

          SIZE_T Offset = 0;
          for (SIZE_T i = 0; i < Scene->InstanceCount; i++)
          {
            VkAccelerationStructureDeviceAddressInfoKHR DeviceAddressInfo
            {
              .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
              .pNext = nullptr,
              .accelerationStructure = Models[i]->BLAS,
            };

            VkDeviceAddress BLASAddres = vkGetAccelerationStructureDeviceAddressKHR(Device, &DeviceAddressInfo);

            InstanceData[i] = VkAccelerationStructureInstanceKHR
            {
              .transform = Models[i]->TransformMatrix,
              .instanceCustomIndex = 0,
              .mask = 0xFF,
              .instanceShaderBindingTableRecordOffset = (UINT32)Offset,
              .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
              .accelerationStructureReference = BLASAddres,
            };
            Offset += Models[i]->Primitives.size();
          }
          Scene->InstanceBuffer.UnmapMemory();

          VkAccelerationStructureGeometryInstancesDataKHR InstancesVk
          {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
            .pNext = nullptr,
            .arrayOfPointers = VK_FALSE,
            .data = { .deviceAddress = Scene->InstanceBuffer.GetDeviceAddress() },
          };

          VkAccelerationStructureGeometryKHR Geometry
          {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
            .pNext = nullptr,
            .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
            .geometry = { .instances = InstancesVk },
            .flags = 0,
          };

          VkAccelerationStructureBuildGeometryInfoKHR BuildGeometryInfo
          {
            /* VkStructureType                                  */ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
            /* const void*                                      */ .pNext = nullptr,
            /* VkAccelerationStructureTypeKHR                   */ .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
            /* VkBuildAccelerationStructureFlagsKHR             */ .flags = 0,
            /* VkBuildAccelerationStructureModeKHR              */ .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
            /* VkAccelerationStructureKHR                       */ .srcAccelerationStructure = VK_NULL_HANDLE,
            /* VkAccelerationStructureKHR                       */ .dstAccelerationStructure = Scene->TLAS,
            /* uint32_t                                         */ .geometryCount = 1,
            /* const VkAccelerationStructureGeometryKHR*        */ .pGeometries = &Geometry,
            /* const VkAccelerationStructureGeometryKHR* const* */ .ppGeometries = nullptr,
            /* VkDeviceOrHostAddressKHR                         */ .scratchData = {},
          };

          VkAccelerationStructureBuildSizesInfoKHR BuildSizesInfo {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};

          VkAccelerationStructureBuildRangeInfoKHR RangeInfo
          {
            .primitiveCount = UINT32(Scene->InstanceCount),
            .primitiveOffset = 0,
            .firstVertex = 0,
            .transformOffset = 0,
          };

          vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &BuildGeometryInfo, &RangeInfo.primitiveCount, &BuildSizesInfo);
          Scene->TLASStorageBuffer = CreateBuffer(BuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

          VkAccelerationStructureCreateInfoKHR CreateInfo
          {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
            .buffer = Scene->TLASStorageBuffer.Buffer,
            .offset = 0,
            .size = BuildSizesInfo.accelerationStructureSize,
            .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
            .deviceAddress = Scene->TLASStorageBuffer.GetDeviceAddress(),
          };

          utils::AssertResult(vkCreateAccelerationStructureKHR(Device, &CreateInfo, nullptr, &Scene->TLAS), "error building TLAS.");

          BuildGeometryInfo.dstAccelerationStructure = Scene->TLAS;
          buffer ScratchBuffer = CreateBuffer(BuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
          BuildGeometryInfo.scratchData.deviceAddress = ScratchBuffer.GetDeviceAddress();

          // build structure
          VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();
          VkAccelerationStructureBuildRangeInfoKHR *PtrRangeInfo = &RangeInfo;
          vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &BuildGeometryInfo, &PtrRangeInfo);
          EndSingleTimeCommands(CommandBuffer);

          Destroy(ScratchBuffer);

          // write pipeline and other shit

          VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[]
          {
            {
              .binding = 0,
              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
              .descriptorCount = 1,
              .stageFlags = VK_SHADER_STAGE_ALL,
            },
            {
              .binding = 1,
              .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
              .descriptorCount = 1,
              .stageFlags = VK_SHADER_STAGE_ALL,
            },
            {
              .binding = 2,
              .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              .descriptorCount = 1,
              .stageFlags = VK_SHADER_STAGE_ALL,
            },
            {
              .binding = 3,
              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
              .descriptorCount = 1,
              .stageFlags = VK_SHADER_STAGE_ALL,
            }
          };

          VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo
          {
            /* VkStructureType                     */ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            /* const void*                         */ .pNext = nullptr,
            /* VkDescriptorSetLayoutCreateFlags    */ .flags = 0,
            /* uint32_t                            */ .bindingCount = (UINT32)std::size(DescriptorSetLayoutBindings),
            /* const VkDescriptorSetLayoutBinding* */ .pBindings = DescriptorSetLayoutBindings,
          };

          utils::AssertResult(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, nullptr, &Scene->DescriptorSetLayout), "error creating descriptor set layout.");

          VkDescriptorPoolSize PoolSizes[]
          {
            { .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,              .descriptorCount = 1, },
            { .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1, },
            { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             .descriptorCount = 1, },
            { .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             .descriptorCount = 1, },
          };

          VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo
          {
            /* VkStructureType             */ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            /* const void*                 */ .pNext = nullptr,
            /* VkDescriptorPoolCreateFlags */ .flags = 0,
            /* uint32_t                    */ .maxSets = 1,
            /* uint32_t                    */ .poolSizeCount = (UINT32)std::size(PoolSizes),
            /* const VkDescriptorPoolSize* */ .pPoolSizes = PoolSizes,
          };

          utils::AssertResult(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, nullptr, &Scene->DescriptorPool), "error creating descriptor pool.");

          VkDescriptorSetAllocateInfo DescriptorSetAllocInfo
          {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = Scene->DescriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &Scene->DescriptorSetLayout,
          };

          utils::AssertResult(vkAllocateDescriptorSets(Device, &DescriptorSetAllocInfo, &Scene->DescriptorSet));

          VkWriteDescriptorSetAccelerationStructureKHR DescriptorSetAccelerationStructure
          {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
            .pNext = nullptr,
            .accelerationStructureCount = 1,
            .pAccelerationStructures = &Scene->TLAS,
          };
          VkDescriptorImageInfo DescriptorSetWriteImageInfo
          {
            .sampler = TargetImageSampler,
            .imageView = TargetImage.ImageView,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
          };
          VkDescriptorBufferInfo GlobalBufferInfo
          {
            .buffer = Scene->GlobalBuffer.Buffer,
            .offset = 0,
            .range = Scene->GlobalBuffer.Size,
          };
          VkDescriptorBufferInfo LightStorageBufferInfo
          {
            .buffer = Scene->LightStorageBuffer.Buffer,
            .offset = 0,
            .range = Scene->LightStorageBuffer.Size,
          };

          VkWriteDescriptorSet DescriptorSetWrites[]
          {
            /* Target binding */
            {
              /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              /* const void*                   */ .pNext = nullptr,
              /* VkDescriptorSet               */ .dstSet = Scene->DescriptorSet,
              /* uint32_t                      */ .dstBinding = 0,
              /* uint32_t                      */ .dstArrayElement = 0,
              /* uint32_t                      */ .descriptorCount = 1,
              /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
              /* const VkDescriptorImageInfo*  */ .pImageInfo = &DescriptorSetWriteImageInfo,
              /* const VkDescriptorBufferInfo* */ .pBufferInfo = nullptr,
              /* const VkBufferView*           */ .pTexelBufferView = nullptr,
            },
            /* TLAS binding */
            {
              /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              /* const void*                   */ .pNext = &DescriptorSetAccelerationStructure,
              /* VkDescriptorSet               */ .dstSet = Scene->DescriptorSet,
              /* uint32_t                      */ .dstBinding = 1,
              /* uint32_t                      */ .dstArrayElement = 0,
              /* uint32_t                      */ .descriptorCount = 1,
              /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
              /* const VkDescriptorImageInfo*  */ .pImageInfo = nullptr,
              /* const VkDescriptorBufferInfo* */ .pBufferInfo = 0,
              /* const VkBufferView*           */ .pTexelBufferView = nullptr,
            },

            /* global uniform buffer */
            {
              /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              /* const void*                   */ .pNext = nullptr,
              /* VkDescriptorSet               */ .dstSet = Scene->DescriptorSet,
              /* uint32_t                      */ .dstBinding = 2,
              /* uint32_t                      */ .dstArrayElement = 0,
              /* uint32_t                      */ .descriptorCount = 1,
              /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              /* const VkDescriptorImageInfo*  */ .pImageInfo = nullptr,
              /* const VkDescriptorBufferInfo* */ .pBufferInfo = &GlobalBufferInfo,
              /* const VkBufferView*           */ .pTexelBufferView = nullptr,
            },
            /* light storage buffer */
            {
              /* VkStructureType               */ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              /* const void*                   */ .pNext = nullptr,
              /* VkDescriptorSet               */ .dstSet = Scene->DescriptorSet,
              /* uint32_t                      */ .dstBinding = 3,
              /* uint32_t                      */ .dstArrayElement = 0,
              /* uint32_t                      */ .descriptorCount = 1,
              /* VkDescriptorType              */ .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
              /* const VkDescriptorImageInfo*  */ .pImageInfo = nullptr,
              /* const VkDescriptorBufferInfo* */ .pBufferInfo = &LightStorageBufferInfo,
              /* const VkBufferView*           */ .pTexelBufferView = nullptr,
            }
          };

          vkUpdateDescriptorSets(Device, (UINT32)std::size(DescriptorSetWrites), DescriptorSetWrites, 0, nullptr);

          module_compilation_info Shader {L"", L"", L"lib"};
          shader_compiler Compiler;
          Compiler.Compile("bin/shaders/start", std::span<module_compilation_info>(&Shader, 1), Device);

          Scene->HitShaderGroupCount = 0;
          for (const model *Model : Models)
            Scene->HitShaderGroupCount += Model->Primitives.size();

          std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos {Scene->HitShaderGroupCount + 2};

          ShaderStageCreateInfos[0] = VkPipelineShaderStageCreateInfo
          {
            /* VkStructureType                  */ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            /* VkShaderStageFlagBits            */ .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            /* VkShaderModule                   */ .module = Shader.Module,
            /* const char*                      */ .pName = "rrs_main",
          };

          ShaderStageCreateInfos[1] = VkPipelineShaderStageCreateInfo
          {
            /* VkStructureType                  */ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            /* VkShaderStageFlagBits            */ .stage = VK_SHADER_STAGE_MISS_BIT_KHR,
            /* VkShaderModule                   */ .module = Shader.Module,
            /* const char*                      */ .pName = "rms_main"
          };

          SIZE_T i = 0;
          std::vector<primitive *> Primitives {Scene->HitShaderGroupCount};
          for (SIZE_T m = 0; m < Models.size(); m++)
            for (SIZE_T p = 0; p < Models[m]->Primitives.size(); p++)
            {
              Primitives[i] = Models[m]->Primitives[p];
              ShaderStageCreateInfos[i + 2] = VkPipelineShaderStageCreateInfo
              {
                /* VkStructureType                  */ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                /* VkShaderStageFlagBits            */ .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                /* VkShaderModule                   */ .module = Models[m]->Primitives[p]->Material->ClosestHitShader,
                /* const char*                      */ .pName = rt_shader::GetModuleTypeEntryPointName(rt_shader::module_type::CLOSEST_HIT),
              };
              i++;
            }

          /* Shader gropus */
          std::vector<VkRayTracingShaderGroupCreateInfoKHR> ShaderGroupCreateInfos {Scene->HitShaderGroupCount + 2};
          ShaderGroupCreateInfos[0] = VkRayTracingShaderGroupCreateInfoKHR
          {
            .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
            .pNext = nullptr,
            .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
            .generalShader = 0,
            .closestHitShader = VK_SHADER_UNUSED_KHR,
            .anyHitShader = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR,
            .pShaderGroupCaptureReplayHandle = VK_NULL_HANDLE,
          };

          ShaderGroupCreateInfos[1] = VkRayTracingShaderGroupCreateInfoKHR
          {
            .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
            .pNext = nullptr,
            .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
            .generalShader = 1,
            .closestHitShader = VK_SHADER_UNUSED_KHR,
            .anyHitShader = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR,
            .pShaderGroupCaptureReplayHandle = VK_NULL_HANDLE,
          };

          for (UINT32 i = 0; i < Scene->HitShaderGroupCount; i++)
            ShaderGroupCreateInfos[(SIZE_T)i + 2] = VkRayTracingShaderGroupCreateInfoKHR
            {
              .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
              .pNext = nullptr,
              .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
              .generalShader = VK_SHADER_UNUSED_KHR,
              .closestHitShader = i + 2,
              .anyHitShader = VK_SHADER_UNUSED_KHR,
              .intersectionShader = VK_SHADER_UNUSED_KHR,
              .pShaderGroupCaptureReplayHandle = VK_NULL_HANDLE,
            };

          VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo
          {
            /* VkStructureType              */ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            /* const void*                  */ .pNext = nullptr,
            /* VkPipelineLayoutCreateFlags  */ .flags = 0,
            /* uint32_t                     */ .setLayoutCount = 1,
            /* const VkDescriptorSetLayout* */ .pSetLayouts = &Scene->DescriptorSetLayout,
            /* uint32_t                     */ .pushConstantRangeCount = 0,
            /* const VkPushConstantRange*   */ .pPushConstantRanges = nullptr,
          };

          utils::AssertResult(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, nullptr, &Scene->PipelineLayout), "Pipeline layout");

          VkRayTracingPipelineCreateInfoKHR PipelineCreateInfo
          {
            /* VkStructureType                                   */ .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
            /* const void*                                       */ .pNext = nullptr,
            /* VkPipelineCreateFlags                             */ .flags = 0,
            /* uint32_t                                          */ .stageCount = static_cast<UINT32>(ShaderStageCreateInfos.size()),
            /* const VkPipelineShaderStageCreateInfo*            */ .pStages = ShaderStageCreateInfos.data(),
            /* uint32_t                                          */ .groupCount = static_cast<UINT32>(ShaderGroupCreateInfos.size()),
            /* const VkRayTracingShaderGroupCreateInfoKHR*       */ .pGroups = ShaderGroupCreateInfos.data(),
            /* uint32_t                                          */ .maxPipelineRayRecursionDepth = 12,
            /* const VkPipelineLibraryCreateInfoKHR*             */ .pLibraryInfo = nullptr,
            /* const VkRayTracingPipelineInterfaceCreateInfoKHR* */ .pLibraryInterface = nullptr,
            /* const VkPipelineDynamicStateCreateInfo*           */ .pDynamicState = nullptr,
            /* VkPipelineLayout                                  */ .layout = Scene->PipelineLayout,
            /* VkPipeline                                        */ .basePipelineHandle = VK_NULL_HANDLE,
            /* int32_t                                           */ .basePipelineIndex = 0,
          };

          utils::AssertResult(vkCreateRayTracingPipelinesKHR(Device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &Scene->Pipeline), "error creating RT pipelines");

          vkDestroyShaderModule(Device, Shader.Module, nullptr);

          SIZE_T GroupCount = ShaderGroupCreateInfos.size();
          // Size of one group, here I can append any additional shit.
          SIZE_T GroupHandleSize = PhysicalDeviceProperties.RTPipelineProperties.shaderGroupHandleSize;

          Scene->SBTAlignedGroupSize = utils::Align(GroupHandleSize, PhysicalDeviceProperties.RTPipelineProperties.shaderGroupBaseAlignment);

          SIZE_T SBTSize = GroupCount * Scene->SBTAlignedGroupSize;

          std::vector<BYTE> ShaderHandleStorage;
          ShaderHandleStorage.resize(SBTSize);

          utils::AssertResult(vkGetRayTracingShaderGroupHandlesKHR(Device, Scene->Pipeline, 0, (UINT32)GroupCount, SBTSize, ShaderHandleStorage.data()));

          buffer SBTStagingBuffer = CreateBuffer
          (
            SBTSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
          );

          Scene->SBTStorageBuffer = CreateBuffer(SBTSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

          /* Aligning data */
          BYTE *Data = reinterpret_cast<BYTE *>(SBTStagingBuffer.MapMemory());

          for (SIZE_T g = 0; g < GroupCount; g++)
          {
            std::memcpy(Data + g * Scene->SBTAlignedGroupSize, ShaderHandleStorage.data() + g * GroupHandleSize, GroupHandleSize);
          }

          for (SIZE_T p = 0; p < Primitives.size(); p++)
          {
            struct primitive_info
            {
              UINT64 VertexBufferPtr;
              UINT64 IndexBufferPtr;
              UINT32 VertexSize;
            } *PrimitiveInfo = reinterpret_cast<primitive_info *>(Data + (p + 2) * Scene->SBTAlignedGroupSize + GroupHandleSize);

            PrimitiveInfo->VertexBufferPtr = Primitives[p]->VertexBuffer.GetDeviceAddress();
            PrimitiveInfo->IndexBufferPtr = Primitives[p]->IndexCount == 0 ? 0 : Primitives[p]->IndexBuffer.GetDeviceAddress();
            PrimitiveInfo->VertexSize = static_cast<UINT32>(Primitives[p]->VertexSize);
          }

          SBTStagingBuffer.UnmapMemory();

          SBTStagingBuffer.CopyTo(Scene->SBTStorageBuffer);

          Destroy(SBTStagingBuffer);

          return Scene;
        } /* CreateScene */

        VkShaderModule CreateShaderModule( std::span<const UINT32> SPIRVCode )
        {
          VkShaderModuleCreateInfo CreateInfo
          {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = static_cast<UINT32>(SPIRVCode.size()),
            .pCode = SPIRVCode.data(),
          };

          VkShaderModule ShaderModule = VK_NULL_HANDLE;
          VkResult Result = vkCreateShaderModule(Device, &CreateInfo, nullptr, &ShaderModule);

          return Result == VK_SUCCESS ? ShaderModule : VK_NULL_HANDLE;
        } /* CreateInfo */


        VOID Initialize( SDL_Window *RenderWindow )
        {
          Window = RenderWindow;

          volkInitialize();
          InitializeInstance();

          utils::Assert(SDL_Vulkan_CreateSurface(Window, Instance, &Surface) == SDL_TRUE, "error initializing surface");
          InitializeDevice();
          InitializeCommandPools();

          InitializePresentResources();
          InitializePresentPipeline();


          ptr<material> PlaneMtl = CreateMaterial("Plane", "bin/shaders/plane");
          ptr<material> TriangleMtl = CreateMaterial("Triangle", "bin/shaders/triangle");
          ptr<material> CowMtl = CreateMaterial("Cow", "bin/shaders/cow");

          vec3 PlaneVtx[]
          {
            {-128, 0, -128},
            { 128, 0, -128},
            {-128, 0,  128},
            { 128, 0,  128},
          };
          UINT32 PlaneIdx[] {0, 1, 2, 1, 2, 3};

          vec3 TriangleVtx[3];
          for (INT i = 0; i < 3; i++)
          {
            FLOAT Angle = static_cast<FLOAT>(mth::PI) * (i * 0.666f + 0.5f);

            TriangleVtx[i] = vec3(std::cos(Angle), std::sin(Angle), 0) + vec3(0, 2, 0);
          }
          ptr<primitive> PlanePrimitive = CreatePrimitive<vec3>(PlaneMtl, PlaneVtx, 0, PlaneIdx);

          ptr<primitive> TrianglePrimitive = CreatePrimitive<vec3>(TriangleMtl, TriangleVtx, 0, {});
          primitive * WorldPrimitives[] {PlanePrimitive, TrianglePrimitive};
          ptr<model> WorldModel = CreateModel(WorldPrimitives);


          topology CowTpl = topology::LoadOBJ("bin/models/cow.obj");
          ptr<primitive> CowPrimitive = CreatePrimitive<small_vertex>(CowMtl, CowTpl.Vertices, 0, CowTpl.Indices);

          primitive *CowPrimitives[] {CowPrimitive};
          ptr<model> CowModel = CreateModel(CowPrimitives);

          model *Models[] {WorldModel, CowModel};
          Scene = CreateScene("Default", Models);

          VkFenceCreateInfo FenceCreateInfo
          {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
          };

          utils::AssertResult(vkCreateFence(Device, &FenceCreateInfo, nullptr, &InFlightFence));

          VkSemaphoreCreateInfo SemaphoreCreateInfo
          {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
          };

          utils::AssertResult(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &ImageAvailableSemaphore));
          utils::AssertResult(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderFinishedSemaphore));

          GraphicsCommandBuffer = CreateCommandBuffer(GraphicsCommandPool);
        } /* Initialize */

        VOID Render( VOID )
        {
          // wait for rendering ended
          VkFence RenderingFences[] {InFlightFence};
          vkWaitForFences(Device, 1, &InFlightFence, VK_TRUE, UINT64_MAX);
          vkResetFences(Device, 1, &InFlightFence);

          // get image
          UINT32 ImageIndex = UINT32_MAX;
          VkResult result = vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, ImageAvailableSemaphore, VK_NULL_HANDLE, &ImageIndex);

          /* no image - no render. */
          if (result != VK_SUCCESS)
            return;

          vkResetCommandBuffer(GraphicsCommandBuffer, 0);

          /* Write output image to descriptor sets */

          VkCommandBufferBeginInfo CommandBufferBeginInfo
          {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
          };

          global_buffer_data *BufferData = reinterpret_cast<global_buffer_data *>(Scene->GlobalBuffer.MapMemory());
          *BufferData = global_buffer_data
          {
            .CameraLocation = Scene->Camera.Location,
            .LightNumber = (UINT32)Scene->Lights.size(),
            .CameraDirection = Scene->Camera.Direction,
            .IsMoved = FALSE,
            .CameraRight = Scene->Camera.Right,
            .FrameIndex = (UINT32)CurrentFrame,
            .CameraUp = Scene->Camera.Up,
            .WidthHeightNear = vec3(Scene->Camera.Width, Scene->Camera.Height, Scene->Camera.Near),
          };
          Scene->GlobalBuffer.UnmapMemory();

          present_buffer_data *PresentBufferData = reinterpret_cast<present_buffer_data *>(PresentBuffer.MapMemory());
          if (DoPresentCollection)
            PresentBufferData->CollectionFrameCount++;
          else
            PresentBufferData->CollectionFrameCount = 1;
          PresentBuffer.UnmapMemory();

          vkBeginCommandBuffer(GraphicsCommandBuffer, &CommandBufferBeginInfo);

          vkCmdBindPipeline(GraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Scene->Pipeline);
          vkCmdBindDescriptorSets(GraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Scene->PipelineLayout, 0, 1, &Scene->DescriptorSet, 0, nullptr);

          VkStridedDeviceAddressRegionKHR RayGenRegion
          {
            .deviceAddress = Scene->SBTStorageBuffer.GetDeviceAddress() + Scene->SBTAlignedGroupSize * 0,
            .stride = Scene->SBTAlignedGroupSize,
            .size = Scene->SBTAlignedGroupSize,
          };
          VkStridedDeviceAddressRegionKHR MissRegion
          {
            .deviceAddress = Scene->SBTStorageBuffer.GetDeviceAddress() + Scene->SBTAlignedGroupSize * 1,
            .stride = Scene->SBTAlignedGroupSize,
            .size = Scene->SBTAlignedGroupSize,
          };
          VkStridedDeviceAddressRegionKHR HitRegion
          {
            .deviceAddress = Scene->SBTStorageBuffer.GetDeviceAddress() + Scene->SBTAlignedGroupSize * 2,
            .stride = Scene->SBTAlignedGroupSize,
            .size = Scene->SBTAlignedGroupSize * Scene->HitShaderGroupCount,
          };

          VkStridedDeviceAddressRegionKHR CallableRegion {};

          vkCmdTraceRaysKHR
          (
            GraphicsCommandBuffer,
            &RayGenRegion,
            &MissRegion,
            &HitRegion,
            &CallableRegion,
            SwapchainImageExtent.width, SwapchainImageExtent.height, 1
          );

          VkMemoryBarrier MemoryBarrier
          {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
          };

          /* 'end raytracing subpass' */
          vkCmdPipelineBarrier(GraphicsCommandBuffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &MemoryBarrier, 0, nullptr, 0, nullptr);

          // Drawing image to target
          VkClearValue ClearValue { 0.0f, 1.0f, 0.0f, 1.0f };

          VkRenderPassBeginInfo BeginInfo
          {
            /* VkStructureType     */ .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            /* const void*         */ .pNext = nullptr,
            /* VkRenderPass        */ .renderPass = PresentRenderPass,
            /* VkFramebuffer       */ .framebuffer = Framebuffers[ImageIndex],
            /* VkRect2D            */ .renderArea = { 0, 0, SwapchainImageExtent.width, SwapchainImageExtent.height },
            /* uint32_t            */ .clearValueCount = 1,
            /* const VkClearValue* */ .pClearValues = &ClearValue,
          };
          vkCmdBeginRenderPass(GraphicsCommandBuffer, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);

          vkCmdBindPipeline(GraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PresentPipeline);

          VkViewport Viewport { 0, 0, (FLOAT)SwapchainImageExtent.width, (FLOAT)SwapchainImageExtent.height, 0.0f, 1.0f };
          vkCmdSetViewport(GraphicsCommandBuffer, 0, 1, &Viewport);
          VkRect2D Scissor { {0, 0}, SwapchainImageExtent };
          vkCmdSetScissor(GraphicsCommandBuffer, 0, 1, &Scissor);
          vkCmdBindDescriptorSets(GraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PresentPipelineLayout, 0, 1, &PresentDescriptorSet, 0, nullptr);
          vkCmdDraw(GraphicsCommandBuffer, 4, 1, 0, 0);

          vkCmdEndRenderPass(GraphicsCommandBuffer);

          vkEndCommandBuffer(GraphicsCommandBuffer);

          // submit commands to queue
          VkSemaphore waitSemaphores[] {ImageAvailableSemaphore};
          VkPipelineStageFlags Flags[] {VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
          VkSemaphore signalSemaphores[] {RenderFinishedSemaphore};

          VkSubmitInfo GraphicsSubmitInfo
          {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = (UINT32)std::size(waitSemaphores),
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = Flags,
            .commandBufferCount = 1,
            .pCommandBuffers = &GraphicsCommandBuffer,
            .signalSemaphoreCount = (UINT32)std::size(signalSemaphores),
            .pSignalSemaphores = signalSemaphores,
          };

          utils::AssertResult(vkQueueSubmit(GraphicsQueue, 1, &GraphicsSubmitInfo, InFlightFence), "can't submit graphics command buffer");

          VkSwapchainKHR swapchains[] {Swapchain};
          VkPresentInfoKHR presentInfo
          {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = (UINT32)std::size(signalSemaphores),
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = (UINT32)std::size(swapchains),
            .pSwapchains = swapchains,
            .pImageIndices = &ImageIndex,
            .pResults = nullptr,
          };

          vkQueuePresentKHR(PresentQueue, &presentInfo);

          CurrentFrame++;
        } /* Render */

        VOID FlushFree( VOID )
        {
          manager<scene, std::string>::FlushFree();
          manager<model>::FlushFree();
          manager<primitive>::FlushFree();
          manager<material, std::string>::FlushFree();
        } /* FlushFree */

        VOID Close( VOID )
        {
          Scene->Release();

          vkDeviceWaitIdle(Device);

          FlushFree();

          // destroy presentation data
          Destroy(TargetImage);
          Destroy(PresentBuffer);
          vkDestroySampler(Device, TargetImageSampler, nullptr);
          vkDestroyRenderPass(Device, PresentRenderPass, nullptr);
          vkDestroyDescriptorSetLayout(Device, PresentDescriptorSetLayout, nullptr);
          vkDestroyDescriptorPool(Device, PresentDescriptorPool, nullptr);
          vkDestroyPipelineLayout(Device, PresentPipelineLayout, nullptr);
          vkDestroyPipeline(Device, PresentPipeline, nullptr);
          vkDestroyFence(Device, InFlightFence, nullptr);
          vkDestroySemaphore(Device, ImageAvailableSemaphore, nullptr);
          vkDestroySemaphore(Device, RenderFinishedSemaphore, nullptr);

          VkCommandPool CommandPools[] {GraphicsCommandPool, TransferCommandPool, ComputeCommandPool};
          for (VkCommandPool CommandPool : CommandPools)
            vkDestroyCommandPool(Device, CommandPool, nullptr);

          for (VkFramebuffer Framebuffer : Framebuffers)
            vkDestroyFramebuffer(Device, Framebuffer, nullptr);
          for (VkImageView ImageView : SwapchainImageViews)
            vkDestroyImageView(Device, ImageView, nullptr);

          vkDestroySwapchainKHR(Device, Swapchain, nullptr);
          vkDestroyDevice(Device, nullptr);
          vkDestroySurfaceKHR(Instance, Surface, nullptr);
          vkDestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
          vkDestroyInstance(Instance, nullptr);
        } /* Close */
      }; /* kernel */
    } /* namespace core */
  } /* namespace render */
} /* namespace vrt */

#endif // !defined __vrt_render_core_h_
