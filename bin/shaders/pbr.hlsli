/**/

#ifndef __pbr_hlsli_
#define __pbr_hlsli_

#include <math_consts.hlsli>

struct material
{
  float3 BaseColor;
  float Metallicness;
  float Roughness;
}; /* material */

float3 GetF0( float3 BaseColor, float Metallicness )
{
  return float3(0.04, 0.04, 0.04) * (1 - Metallicness) + BaseColor * Metallicness;
} /* GetF0 */

float3 FrenselSchlick( float3 F0, float HV )
{
  return F0 + (1.0 - F0) * pow(1 - saturate(HV), 5);
} /* FrenselSchlick */

float DistributionTrowbridgeReitzGGX( float NH, float Roughness )
{
  float Roughness2 = Roughness * Roughness;
  float Den = (NH * NH) * (Roughness2 - 1) + 1;

  return Roughness2 / (PI * Den * Den);
} /* DistributionTrowbridgeReitzGGX */

float GeometrySchlickGGX( float Cos, float K )
{
  return Cos / (Cos * (1 - K) + K);
} /* GGX_Schlick */

float3 BRDF_CookTorrance( float3 N, float3 V, float3 L, material Material )
{
  float NL = dot(N, L);
  if (NL < 0)
    return 0;
  float NV = dot(N, V);
  if (NV < 0)
    return 0;
  float3 H = normalize(V + L);
  float NH = dot(N, H);
  float HV = dot(H, V);
  
  float3 F0 = GetF0(Material.BaseColor, Material.Metallicness);
  float3 F = FrenselSchlick(F0, HV);

  float K = pow(Material.Roughness + 1, 2) / 8;
  float D = DistributionTrowbridgeReitzGGX(NH, Material.Roughness);
  float G = GeometrySchlickGGX(NV, K) * GeometrySchlickGGX(NL, K);

  float3 Specular = D * F * G / (4 * NL * NV);
  float3 Diffuse = Material.BaseColor / PI * (1 - Material.Metallicness);

  return Specular + Diffuse;
} /* BRDF_CookTorrance */

float3 PBR_Shade( float3 Position, float3 Normal, float3 CameraPosition, float3 LightPosition, float3 LightColor, material Material )
{
  float3 ViewDirection = normalize(CameraPosition - Position);
  float3 LightDirection = normalize(LightPosition - Position);
  float3 ViewNormal = faceforward(Normal, -ViewDirection, Normal);

  float3 BRDF = BRDF_CookTorrance(ViewNormal, ViewDirection, LightDirection, Material);
  float3 Light = LightColor / distance(Position, LightPosition);
  float3 NL = dot(ViewNormal, LightDirection);

  return BRDF * Light * saturate(NL);
} /* PBR_Shade */

#endif /* !defined __pbr_hlsli_ */
