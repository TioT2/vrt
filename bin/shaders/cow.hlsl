/**/


#include <common.hlsl>

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.Color = float3(Attributes.barycentrics.xy, 0.0);
  Payload.HitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
} /* rcs_main */