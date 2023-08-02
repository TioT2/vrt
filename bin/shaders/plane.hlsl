/**/

#include <common.hlsli>

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.HitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

  bool Cell = bool(int(floor(Payload.HitPosition.x / 2.0) + floor(Payload.HitPosition.z / 2.0)) & 1);

  Payload.HitNormal = float3(0, 1, 0);
  if (Cell)
  {
    Payload.Color = float3(0.30, 0.80, 0.47);
    Payload.Metallicness = 0.00;
    Payload.Roughness = 1.00;
  }
  else
  {
    Payload.Color = float3(1.00, 1.00, 1.00);
    Payload.Metallicness = 1.00;
    Payload.Roughness = 0.1;
  }
} /* rcs_main */
