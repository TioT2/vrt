/**/

#include <common.hlsli>
#include <pbr.hlsli>

float3 RayDirectionFromScreenCoord( float2 ScreenCoord )
{
  return normalize(Camera.DirectionNear.xyz + Camera.RightWidth.xyz * ScreenCoord.x + Camera.UpHeight.xyz * ScreenCoord.y);
} /* RayDirectionFromTexCoord */

float3 Trace( RayDesc Ray )
{
  const float3 LightDirection = normalize(float3(1, 1, 1));
  ray_payload Payload;

  Payload.Color = float3(0, 0, 0);
  Payload.DoHit = true;
  Payload.RecursionDepth = 1;
  Payload.HitNormal = LightDirection;

  TraceRay(Scene, 0, ~0, 0, 1, 0, Ray, Payload);

  if (Payload.DoHit)
  {
    return
      PBR_Shade(Payload.HitPosition, Payload.HitNormal, Camera.Location.xyz, float3(-16, 16, 16), float3(100, 50, 000), Payload.Color, Payload.Metallicness, Payload.Roughness) +
      PBR_Shade(Payload.HitPosition, Payload.HitNormal, Camera.Location.xyz, float3(16, 16, 16),  float3(50, 50, 50), Payload.Color, Payload.Metallicness, Payload.Roughness) +
      PBR_Shade(Payload.HitPosition, Payload.HitNormal, Camera.Location.xyz, float3(16, 16, -16), float3(000, 50, 100), Payload.Color, Payload.Metallicness, Payload.Roughness);
  }
  return Payload.Color;
} /* Trace */

[shader("raygeneration")]
void rrs_main( void )
{
  float2 PixelSize = float2(1, 1) / (float2)DispatchRaysDimensions();
  float2 FragCoord = ((float2)DispatchRaysIndex() + 0.5) * PixelSize;

  // vulkan...
  FragCoord.y = 1 - FragCoord.y;
  FragCoord = FragCoord * 2.0 - 1.0;

  RayDesc Ray;
  Ray.Origin = Camera.Location.xyz;
  Ray.Direction = RayDirectionFromScreenCoord(FragCoord);

  Ray.TMin = 0;
  Ray.TMax = 100;

  float3 Color = float3(0, 0, 0);
  for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++)
    {
      RayDesc SubRay = Ray;

      SubRay.Direction = RayDirectionFromScreenCoord(FragCoord + PixelSize / 3.0 * float2(x, y));
      
      Color += Trace(SubRay);
    }

  Target[DispatchRaysIndex().xy] = float4(Color / 9, 1);
} /* rrs_main */

[shader("miss")]
void rms_main( inout ray_payload Payload )
{
  Payload.Color = 0;
  Payload.DoHit = false;
} /* rms_main */
