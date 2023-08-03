/**/

#include <common.hlsli>

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.HitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
  Payload.HitNormal = float3(0, 0, 1);
  Payload.DoHit = true;

  Payload.BaseColor = float3(1 - Attributes.barycentrics.x - Attributes.barycentrics.y, Attributes.barycentrics.x, Attributes.barycentrics.y);
  Payload.Metallicness = 0.00;
  Payload.Roughness = 1.00;
} /* rcs_main */
