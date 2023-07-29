/**/

struct hit_info
{
  float HitSky;
};

RWTexture2D<float4> Target : register(u0, space0);
RaytracingAccelerationStructure Scene : register(t1, space0);

struct buffer_data
{
  float4 CallCount;
};

typedef BuiltInTriangleIntersectionAttributes attributes;
struct ray_payload
{
    float4 color;
};

[shader("raygeneration")]
void rrs_main( void )
{
  float2 FragCoord = (float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions();

  FragCoord.y = 1 - FragCoord.y;

  ray_payload Payload;
  Payload.color = float4(0, 0, 0, 0);

  RayDesc Ray;
  Ray.Origin = float3(FragCoord, 1);
  Ray.Direction = float3(0, 0, -1);

  Ray.TMin = 0;
  Ray.TMax = 100;

  TraceRay(Scene, 0, ~0, 0, 1, 0, Ray, Payload);

  Target[DispatchRaysIndex().xy] = Payload.color;
} /* rrs_main */

[shader("miss")]
void rms_main( inout ray_payload Payload )
{
  Payload.color = float4(0.30, 0.47, 0.80, 0.0);
} /* rms_main */

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.color = float4(0.0, 1.0, 0.0, 0.0);
} /* rcs_main */