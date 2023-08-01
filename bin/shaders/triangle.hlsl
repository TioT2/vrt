/**/

#include <common.hlsl>

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.Color = float3(1 - Attributes.barycentrics.x - Attributes.barycentrics.y, Attributes.barycentrics.x, Attributes.barycentrics.y);
  Payload.HitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
} /* rcs_main */