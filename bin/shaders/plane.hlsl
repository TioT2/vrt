/**/

#include <common.hlsli>

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.HitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
  Payload.DoHit = true;

  bool Cell = bool(int(floor(Payload.HitPosition.x / 2.0) + floor(Payload.HitPosition.z / 2.0)) & 1);

  Payload.HitNormal = float3(0, 1, 0);
  if (Cell)
  {
    Payload.Material.BaseColor = float3(0.30, 0.80, 0.47);
    Payload.Material.Metallicness = 0.00;
    Payload.Material.Roughness = 1.00;
  }
  else
  {
    Payload.Material.BaseColor = 1.00;
    Payload.Material.Metallicness = 1.00;
    Payload.Material.Roughness = 0.1;
  }
} /* rcs_main */
