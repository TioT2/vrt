/**/


#include <common.hlsli>

[[vk::shader_record_ext]] ConstantBuffer<primitive_info> PrimitiveInfo;

struct input_vertex
{
  float PTN[8];

  float3 Position( void ) { return float3(PTN[0], PTN[1], PTN[2]); }
  float2 TexCoord( void ) { return float2(PTN[3], PTN[4]); }
  float3 Normal  ( void ) { return float3(PTN[5], PTN[6], PTN[7]); }
};

struct vertex
{
  float3 Position;
  float2 TexCoord;
  float3 Normal;
};

float3 GetNormalIndexed( float3 B )
{
  int FirstIndex = PrimitiveIndex() * 3;

  int
    i0 = vk::RawBufferLoad<uint32_t>(PrimitiveInfo.IndexBufferPtr + 4 * (FirstIndex + 0)),
    i1 = vk::RawBufferLoad<uint32_t>(PrimitiveInfo.IndexBufferPtr + 4 * (FirstIndex + 1)),
    i2 = vk::RawBufferLoad<uint32_t>(PrimitiveInfo.IndexBufferPtr + 4 * (FirstIndex + 2));

  input_vertex
    v0 = vk::RawBufferLoad<input_vertex>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * i0),
    v1 = vk::RawBufferLoad<input_vertex>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * i1),
    v2 = vk::RawBufferLoad<input_vertex>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * i2);

  return normalize(v0.Normal() * (1 - B.x - B.y) + v1.Normal() * B.x + v2.Normal() * B.y);
} /* GetVertex */

vertex GetVertex( float3 B )
{
  uint64_t FirstIndex = PrimitiveIndex() * 3;

  input_vertex
    v0 = vk::RawBufferLoad<input_vertex>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * (FirstIndex + 0)),
    v1 = vk::RawBufferLoad<input_vertex>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * (FirstIndex + 1)),
    v2 = vk::RawBufferLoad<input_vertex>(PrimitiveInfo.VertexBufferPtr + PrimitiveInfo.VertexSize * (FirstIndex + 2));
  vertex v;
  v.Position = v0.Position() * B.x + v1.Position() * B.y + v2.Position() * B.z;
  v.TexCoord = v0.TexCoord() * B.x + v1.TexCoord() * B.y + v2.TexCoord() * B.z;
  v.Normal   = v0.Normal()   * B.x + v1.Normal()   * B.y + v2.Normal()   * B.z;

  return v;
} /* GetVertex */

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.HitNormal = GetVertex(float3(1 - Attributes.barycentrics.x - Attributes.barycentrics.y, Attributes.barycentrics.xy)).Normal;
  Payload.Color = float3(1, 1, 1);
  Payload.HitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
} /* rcs_main */
