/**/


#include <common.hlsli>

[[vk::shader_record_ext]] ConstantBuffer<primitive_info> PrimitiveInfo;

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  uint64_t FirstIndexIndex = PrimitiveIndex() * 3;

  uint32_t
    Index0 = vk::RawBufferLoad<uint32_t>(PrimitiveInfo.IndexBufferPtr + 4 * (FirstIndexIndex + 0)),
    Index1 = vk::RawBufferLoad<uint32_t>(PrimitiveInfo.IndexBufferPtr + 4 * (FirstIndexIndex + 1)),
    Index2 = vk::RawBufferLoad<uint32_t>(PrimitiveInfo.IndexBufferPtr + 4 * (FirstIndexIndex + 2));

  float3
    P0 = vk::RawBufferLoad<float3>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * Index0),
    P1 = vk::RawBufferLoad<float3>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * Index1),
    P2 = vk::RawBufferLoad<float3>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * Index2);

  Payload.HitNormal = normalize(cross(P1 - P0, P2 - P0));
  Payload.HitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
  Payload.DoHit = true;

  Payload.Material.BaseColor = float3(1, 1, 1);
  Payload.Material.Roughness = 0.3;
  Payload.Material.Metallicness = 1.0;
} /* rcs_main */
