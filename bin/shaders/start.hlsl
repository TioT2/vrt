/**/

#include <common.hlsl>

float3 RayDirectionFromScreenCoord( float2 ScreenCoord )
{
  return normalize(Camera.DirectionNear.xyz + Camera.RightWidth.xyz * ScreenCoord.x + Camera.UpHeight.xyz * ScreenCoord.y);
} /* RayDirectionFromTexCoord */

[shader("raygeneration")]
void rrs_main( void )
{
  float2 PixelSize = float2(1, 1) / (float2)DispatchRaysDimensions();
  float2 FragCoord = (float2)DispatchRaysIndex() * PixelSize;

  // vulkan...
  FragCoord.y = 1 - FragCoord.y;
  FragCoord = FragCoord * 2.0 - 1.0;

  RayDesc Ray;
  Ray.Origin = Camera.Location.xyz;
  Ray.Direction = RayDirectionFromScreenCoord(FragCoord);//float3(0, 0, -1);

  Ray.TMin = 0;
  Ray.TMax = 100;

  // some MSAA
  float4 Color = float4(0, 0, 0, 0);
  for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++)
    {
      RayDesc SubRay = Ray;
      ray_payload Payload;

      Payload.Color = float4(0, 0, 0, 0);
      SubRay.Origin = Camera.Location.xyz;
      SubRay.Direction = RayDirectionFromScreenCoord(FragCoord + PixelSize / 3.0 * float2(x, y));

      TraceRay(Scene, 0, ~0, 0, 1, 0, SubRay, Payload);

      Color += Payload.Color;
    }

  Target[DispatchRaysIndex().xy] = Color / 9;
} /* rrs_main */

[shader("miss")]
void rms_main( inout ray_payload Payload )
{
  Payload.Color = float4(0.30, 0.47, 0.80, 0.0);
} /* rms_main */