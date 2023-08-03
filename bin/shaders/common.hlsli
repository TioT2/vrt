/**/

#ifndef __common_hlsli_
#define __common_hlsli_

#include <pbr.hlsli>

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
  /*
    light Lights[3];

    Lights[0].Position = float3(16, 16, 16);
    Lights[0].Color    = float3(50, 50, 50);

    Lights[1].Position = float3(-16, 16, 16);
    Lights[1].Color    = float3(100, 50, 0);

    Lights[2].Position = float3(16, 16, -16);
    Lights[2].Color    = float3(000, 50, 100);
*/
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

#endif // !defined __common_hlsli_
