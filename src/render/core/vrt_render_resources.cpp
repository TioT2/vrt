#include "vrt.h"

namespace vrt::render::core
{
  material::~material( VOID )
  {
    if (ClosestHitShader != VK_NULL_HANDLE) vkDestroyShaderModule(Kernel->Device, ClosestHitShader, nullptr);
  } /* ~material */

  model::~model( VOID )
  {
    vkDestroyAccelerationStructureKHR(Kernel->Device, BLAS, nullptr);
    Kernel->Destroy(BLASStorageBuffer);
    for (primitive *Prim : Primitives)
      Prim->Release();
  } /* model */

  primitive::~primitive( VOID )
  {
    Material->Release();
    Kernel->Destroy(VertexBuffer);
    if (IndexCount != 0)
      Kernel->Destroy(IndexBuffer);
  } /* ~primitive */

  /* Scene destructor */
  scene::~scene( VOID )
  {
    for (model *Model : Models)
      Model->Release();

    vkDestroyAccelerationStructureKHR(Kernel->Device, TLAS, nullptr);
    vkDestroyDescriptorPool(Kernel->Device, DescriptorPool, nullptr);
    vkDestroyPipeline(Kernel->Device, Pipeline, nullptr);
    vkDestroyPipelineLayout(Kernel->Device, PipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(Kernel->Device, DescriptorSetLayout, nullptr);

    Kernel->Destroy(InstanceBuffer);
    Kernel->Destroy(TLASStorageBuffer);
    Kernel->Destroy(SBTStorageBuffer);
  } /* ~scene */
} /* vrt::render::core */