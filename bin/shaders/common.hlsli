/**/

#ifndef __common_hlsli_
#define __common_hlsli_

#include <pbr.hlsli>
#include <rand.hlsli>

struct hit_info
{
  float HitSky;
};

struct buffer_data
{
  float3 CameraLocation;
  uint32_t LightNumber;
  float3 CameraDirection;
  bool IsMoved;
  float3 CameraRight;
  uint32_t FrameIndex;
  float3 CameraUp;
}; /* buffer_data */

struct point_light
{
  float3 Position;
  float3 Color;
  float Aligner;
}; /* point_light */

RWTexture2D<float4> Target : register(u0, space0);
RaytracingAccelerationStructure Scene : register(t1, space0);
ConstantBuffer<buffer_data> GlobalBuffer : register(b2, space0);
StructuredBuffer<point_light> Lights : register(t3, space0);

typedef BuiltInTriangleIntersectionAttributes attributes;

struct [raypayload] ray_payload
{
  // basic values
  int RecursionDepth;

  // hit info
  float3 HitPosition;
  bool DoHit;
  float3 HitNormal;

  material Material;
};

/* PerPrimitive data */
struct primitive_info
{
  uint64_t VertexBufferPtr;
  uint64_t IndexBufferPtr;
  uint32_t VertexSize;
}; /* primitive_info */

float3 Trace( RayDesc Ray, float RecursionDepth = 0 )
{
  ray_payload Payload;

  if (RecursionDepth > 12)
    return 0;

  Payload.RecursionDepth = RecursionDepth + 1;

  TraceRay(Scene, 0, ~0, 0, 1, 0, Ray, Payload);

  if (Payload.DoHit)
  {
    float3 Color = 0;
    for (int i = 0; i < GlobalBuffer.LightNumber; i++)
    {
      RayDesc ShadowRay = Ray;

      ShadowRay.Direction = normalize(Lights[i].Position - Payload.HitPosition);
      ShadowRay.Origin = Payload.HitPosition + 0.001 * ShadowRay.Direction;

      ray_payload ShadowPayload;

      ShadowPayload.RecursionDepth = RecursionDepth + 1;
      TraceRay(Scene, 0, ~0, 0, 1, 0, ShadowRay, ShadowPayload);

      if (!ShadowPayload.DoHit)
        Color += PBR_Shade(Payload.HitPosition, Payload.HitNormal, Ray.Origin, Lights[i].Position, Lights[i].Color, Payload.Material);
    }

    return Color;
  }
  return Payload.Material.BaseColor;
} /* Trace */

#if 0
float3 CosHemisphereRandom( float3 Normal )
{
  // Normal is up
  float3 Right = float3(1, 0, 0);
  float3 Dir = normalize(cross(Normal, Right));
  Right = normalize(cross(Dir, Normal));

  float3 Coord;
  Coord.xz = float2(Rand(Normal.x, GlobalBuffer.FrameIndex * 30.47), Rand(Normal.y, GlobalBuffer.FrameIndex * 30.47));
  Coord.y = sqrt(1 - Coord.x * Coord.x - Coord.z * Coord.z);

  return Right * Coord.x + Normal * Coord.y + Dir * Coord.z;
} /* CosHemisphereRandom */

float3 DirectCos( float3 P, float3 N, float3 ViewDirection, material Material, int RecursionDepth = 0 )
{
  float3 WI = CosHemisphereRandom(N);
  float pdf = dot(WI, N) / PI;

  ray_payload Payload;

  Payload.RecursionDepth = RecursionDepth + 1;

  RayDesc Ray;
  Ray.Origin = P;
  Ray.Direction = ViewDirection;
  Ray.TMin = 0;
  Ray.TMax = 1024.0;

  TraceRay(Scene, 0, ~0, 0, 1, 0, Ray, Payload);

  if (!Payload.DoHit)
    return vec3(0.0);

  float3 BRDF = BRDF_CookTorrance(N, ViewDirection, WI, Material);
  float3 Le = float3(1, 1, 1);//evaluate_emissive(i, WI);

  return BRDF * dot(WI, N) * Le / pdf;
} /* direct_cos */
#endif // 0

#endif // !defined __common_hlsli_
