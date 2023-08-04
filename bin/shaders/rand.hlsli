/**/

#ifndef __rand_hlsli_
#define __rand_hlsli_

#include <math_consts.hlsli>

float NRand( float N )
{
  return max(frac(sin(N.x * 12.9898) * 43758.5453), 0.001);
} /* NRand */

float InvErrorFunction( float X )
{
  const float ALPHA = 0.14;
  const float INV_ALPHA = 1.0 / ALPHA;
  const float K = 2.0 / (PI * ALPHA);

  float Y = log(1.0 - X * X);
  float Z = K + 0.5 * Y;
  return sqrt(sqrt(Z * Z - Y * INV_ALPHA) - Z) * sign(X);
} /* InvErrorFunction */

float Rand( float N, float Seed = 47 )
{
  float T = frac(cos(Seed));
  float X = NRand(N + 0.07 * T);

  return InvErrorFunction(X * 2.0 - 1.0) * 0.15 + 0.5;
} /* Rand */

#endif // !defined __rand_hlsli_