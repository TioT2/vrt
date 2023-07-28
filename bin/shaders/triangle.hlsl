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

  Target[DispatchRaysIndex().xy] = float4(FragCoord, 0.0, 1.0);
} /* rrs_main */

[shader("miss")]
void rms_main( inout ray_payload Payload )
{

} /* rms_main */

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{

} /* rcs_main */