/**/

#ifndef __common_hlsli_
#define __common_hlsli_

#include <pbr.hlsli>

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
  // basic values
  int RecursionDepth;

  // hit info
  float3 HitPosition;
  bool DoHit;
  float3 HitNormal;

  // material
  float Metallicness;
  float3 BaseColor;
  float Roughness;
};

/* PerPrimitive data */
struct primitive_info
{
  uint64_t VertexBufferPtr;
  uint64_t IndexBufferPtr;
  uint32_t VertexSize;
}; /* primitive_info */

struct light
{
  float3 Position;
  float3 Color;
};

float3 Trace( RayDesc Ray, float RecursionDepth = 1 )
{
  const float3 LightDirection = normalize(float3(1, 1, 1));
  ray_payload Payload;

  if (RecursionDepth >= 8)
    return 0;

  Payload.RecursionDepth = RecursionDepth;

  TraceRay(Scene, 0, ~0, 0, 1, 0, Ray, Payload);

  if (Payload.DoHit)
  {
    light Lights[3];

    Lights[0].Position = float3(16, 16, 16);
    Lights[0].Color    = float3(50, 50, 50);

    Lights[1].Position = float3(-16, 16, 16);
    Lights[1].Color    = float3(100, 50, 0);

    Lights[2].Position = float3(16, 16, -16);
    Lights[2].Color    = float3(000, 50, 100);

    float3 Color = 0;
    for (int i = 0; i < 3; i++)
    {
      RayDesc ShadowRay = Ray;

      ShadowRay.Direction = normalize(Lights[i].Position - Payload.HitPosition);
      ShadowRay.Origin = Payload.HitPosition + 0.001 * ShadowRay.Direction;

      ray_payload ShadowPayload;

      TraceRay(Scene, 0, ~0, 0, 1, 0, ShadowRay, ShadowPayload);

      if (!ShadowPayload.DoHit)
        Color += PBR_Shade(Payload.HitPosition, Payload.HitNormal, Ray.Origin, Lights[i].Position, Lights[i].Color, Payload.BaseColor, Payload.Metallicness, Payload.Roughness);
    }
    return Color;
  }
  return Payload.BaseColor;
} /* Trace */

#endif // !defined __common_hlsli_
