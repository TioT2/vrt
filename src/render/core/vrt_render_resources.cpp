#include "vrt.h"

namespace vrt::render::core
{
  primitive::~primitive( VOID )
  {
    vkDestroyAccelerationStructureKHR(Kernel->Device, TLAS, nullptr);
    Kernel->Destroy(TLASStorageBuffer);
    Kernel->Destroy(VertexBuffer);
    if (IndexCount != 0)
      Kernel->Destroy(IndexBuffer);
  } /* ~primitive */

  /* Scene destructor */
  scene::~scene( VOID )
  {
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