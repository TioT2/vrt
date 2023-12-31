/**/

#include <common.hlsli>
#include <pbr.hlsli>

float3 RayDirectionFromScreenCoord( float2 ScreenCoord )
{
  return normalize(GlobalBuffer.CameraDirection * GlobalBuffer.WidthHeightNear.z + GlobalBuffer.CameraRight * ScreenCoord.x + GlobalBuffer.CameraUp * ScreenCoord.y);
} /* RayDirectionFromTexCoord */

[shader("raygeneration")]
void rrs_main( void )
{
  float2 PixelSize = float2(1, 1) / (float2)DispatchRaysDimensions();
  float2 FragCoord = ((float2)DispatchRaysIndex() + 0.5) / (float2)DispatchRaysDimensions();

  FragCoord = FragCoord * 2.0 - 1.0;            // [0...1] -> [-1...1]
  FragCoord.y = -FragCoord.y;                   // vulkan...
  FragCoord *= GlobalBuffer.WidthHeightNear.xy; // aspect

  RayDesc Ray;
  Ray.Origin = GlobalBuffer.CameraLocation.xyz;

  float2 FragCoordOffset = float2
  (
    Rand(FragCoord.x, GlobalBuffer.FrameIndex * 30.47),
    Rand(FragCoord.y, GlobalBuffer.FrameIndex * 30.47)
  ).yx * 2 - 1;

  Ray.Direction = RayDirectionFromScreenCoord(FragCoord + FragCoordOffset * PixelSize * 5.0);

  Ray.TMin = 0;
  Ray.TMax = 1024;

  Target[DispatchRaysIndex().xy] += float4(Trace(Ray), 0);
} /* rrs_main */

[shader("miss")]
void rms_main( inout ray_payload Payload )
{
  Payload.Material.BaseColor = 0;
  Payload.DoHit = false;
} /* rms_main */
