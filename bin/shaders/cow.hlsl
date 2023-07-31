/**/


#include <common.hlsl>

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.Color = float4(1.00, 1.00, 1.00, /*1 - Attributes.barycentrics.x - Attributes.barycentrics.y, Attributes.barycentrics.x, Attributes.barycentrics.y,*/ 0.0);
} /* rcs_main */