/**/

// Target texture

#include <rand.hlsli>

struct present_buffer_data
{
  uint32_t CollectionFrameCount;
};

RWTexture2D<float4> Texture : register(u0, space0);
ConstantBuffer<present_buffer_data> PresentBuffer : register(b1, space0);

struct vs_out
{
  float4 Position : SV_Position;
  float2 TexCoord : TEXCOORD0;
};

vs_out vs_main( const int VertexID : SV_VertexID )
{
  vs_out Out;

  Out.Position = float4(VertexID < 2 ? -1 : 1, (VertexID % 2) * 2 - 1, 0.0, 0.5);
  Out.TexCoord = (Out.Position.xy + 1.0) / 2.0;

  return Out;
} /* vs_main */

float4 ps_main( const vs_out In ) : SV_Target0
{
  float4 Value = Texture[(uint2)In.Position.xy].xyzw;

  if (PresentBuffer.CollectionFrameCount == 1)
    Texture[(uint2)In.Position.xy].xyzw = 0;

  Value /= PresentBuffer.CollectionFrameCount;

  return Value / (Value + 1); /* TONE MAPPING JOPTIT' */
} /* ps_main */
