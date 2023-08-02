/**/

#ifndef __common_hlsli_
#define __common_hlsli_

struct hit_info
{
  float HitSky;
};

struct camera_data
{
  float4 Location;
  float4 DirectionNear;
  float4 RightWidth;
  float4 UpHeight;
};

RWTexture2D<float4> Target : register(u0, space0);
RaytracingAccelerationStructure Scene : register(t1, space0);
ConstantBuffer<camera_data> Camera : register(b2, space0);

typedef BuiltInTriangleIntersectionAttributes attributes;

struct [raypayload] ray_payload
{
  float3 Color;
  int RecursionDepth;
  float3 HitPosition;
  bool DoHit;
  float3 HitNormal;
};

/* PerPrimitive data */
struct primitive_info
{
  uint64_t VertexBufferPtr;
  uint64_t IndexBufferPtr;
  uint32_t VertexSize;
}; /* primitive_info */

#endif // !defined __common_hlsli_
