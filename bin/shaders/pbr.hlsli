/**/

#ifndef __pbr_hlsli_
#define __pbr_hlsli_

#include <math_consts.hlsli>

float3 GetF0( float3 BaseColor, float Metalness )
{
  return float3(0.04, 0.04, 0.04) * (1 - Metalness) + BaseColor * Metalness;
} /* GetF0 */

float3 FrenselSchlick( float3 F0, float HV )
{
  return F0 + (1.0 - F0) * pow(1 - HV, 5);
} /* FrenselSchlick */

float GGX_TrowbridgeReitz( float NH, float Roughness )
{
  float Roughness2 = Roughness * Roughness;

  return Roughness2 / (PI * pow((NH * NH) * (Roughness2 - 1) + 1, 2));
} /* GGX_TrowbridgeReitz */

float GGX_Schlick( float NV, float K )
{
  return NV / (NV * (1 - K) + K);
} /* GGX_Schlick */

float3 BRDF_CookTorrance( float3 N, float3 V, float3 L, float3 BaseColor, float Metalness, float Roughness )
{
  float NV = dot(N, V);
  if (NV < 0)
    return float3(0, 0, 0);


  float3 H = normalize(V + L);
  
  float3 F0 = GetF0(BaseColor, Metalness);
  float3 F = FrenselSchlick(F0, Metalness);

  float K = pow(Roughness + 1, 2) / 8;
  float D = GGX_TrowbridgeReitz(NH, Roughness);
  float G = GGX_Schlick(NV, K) * GGX_Schlick(NL, K);

  return
    F * D * G / (4 * NV * NL) +
    (1 - F) * BaseColor / PI * (1 - Metalness);
} /* BRDF_CookTorrance */

float3 PBR_Shade( float3 Position, float3 LightPosition, )

#endif /* !defined __pbr_hlsli_ */
