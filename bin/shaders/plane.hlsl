/**/

#include <common.hlsl>

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.HitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

  Payload.Color = float4(0.30, 0.80, 0.47, 1.00) * float(int(floor(Payload.HitPosition.x) + floor(Payload.HitPosition.z)) & 1);
} /* rcs_main */