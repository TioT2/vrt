/**/

// Target texture

[[vk::binding(0, 0)]] RWTexture2D<float4> Texture;
// [[vk::binding(0, 1)]] SamplerState Sampler;

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
  return Texture[(uint2)In.Position.xy].xyzw; /* yey, das't all i wanna do */
} /* ps_main */