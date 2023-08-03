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
    if (Payload.RecursionDepth < 4)
    {
      RayDesc Ray;
    
      Ray.TMin = 0;
      Ray.TMax = 1024;
      Ray.Origin = Payload.HitPosition + float3(0, 0.0001, 0);
      Ray.Direction = reflect(normalize(Payload.HitPosition - Camera.Location.xyz), float3(0, 1, 0));
    
      Payload.BaseColor = Trace(Ray, Payload.RecursionDepth + 1);
    }
    else
      Payload.BaseColor = float3(0.30, 0.80, 0.47);

    Payload.Metallicness = 0.00;
    Payload.Roughness = 1.00;
  }
  else
  {
    Payload.BaseColor = 1.00;
    Payload.Metallicness = 1.00;
    Payload.Roughness = 0.1;
  }
} /* rcs_main */
