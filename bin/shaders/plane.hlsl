/**/

#include <common.hlsl>

[shader("closesthit")]
void rcs_main( inout ray_payload Payload, in attributes Attributes )
{
  Payload.Color = float4(0.30, 0.80, 0.47, 1.00);
} /* rcs_main */