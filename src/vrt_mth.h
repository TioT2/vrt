#ifndef __vrt_mth_h_
#define __vrt_mth_h_

#include "vrt_def.h"

#define VRT_MTH_DEFINE_OPERATOR_VEC3_VEC3(Oper)            vec3<component> operator Oper  ( const vec3<component> &rhs ) const { return vec3<component>(X Oper rhs.X, Y Oper rhs.Y, Z Oper rhs.Z); }
#define VRT_MTH_DEFINE_ASSIGNMENT_OPERATOR_VEC3_VEC3(Oper) vec3<component> & operator Oper##=( const vec3<component> &rhs )       { X Oper##= rhs.X; Y Oper##= rhs.Y; Z Oper##= rhs.Z; return *this; }
#define VRT_MTH_DEFINE_OPERATOR_VEC3_COMPONENT(Oper)            vec3<component> operator Oper  ( component rhs ) const { return vec3<component>(X Oper rhs, Y Oper rhs, Z Oper rhs); }
#define VRT_MTH_DEFINE_ASSIGNMENT_OPERATOR_VEC3_COMPONENT(Oper) vec3<component> & operator Oper##=( component rhs ) { X Oper##= rhs; Y Oper##= rhs; Z Oper##= rhs; return *this; }

#define VRT_MTH_DEFINE_OPERATOR_PACK_VEC3(Oper)           \
  VRT_MTH_DEFINE_OPERATOR_VEC3_VEC3(Oper)                 \
  VRT_MTH_DEFINE_ASSIGNMENT_OPERATOR_VEC3_VEC3(Oper)      \
  VRT_MTH_DEFINE_OPERATOR_VEC3_COMPONENT(Oper)            \
  VRT_MTH_DEFINE_ASSIGNMENT_OPERATOR_VEC3_COMPONENT(Oper) \


#define VRT_MTH_DEFINE_OPERATOR_VEC2_VEC2(Oper)            vec2<component> operator Oper  ( const vec2<component> &rhs ) const { return vec3<component>(X Oper rhs.X, Y Oper rhs.Y); }
#define VRT_MTH_DEFINE_ASSIGNMENT_OPERATOR_VEC2_VEC2(Oper) vec2<component> & operator Oper##=( const vec2<component> &rhs )       { X Oper##= rhs.X; Y Oper##= rhs.Y; return *this; }
#define VRT_MTH_DEFINE_OPERATOR_VEC2_COMPONENT(Oper)            vec2<component> operator Oper  ( component rhs ) const { return vec3<component>(X Oper rhs, Y Oper rhs); }
#define VRT_MTH_DEFINE_ASSIGNMENT_OPERATOR_VEC2_COMPONENT(Oper) vec2<component> & operator Oper##=( component rhs ) { X Oper##= rhs; Y Oper##= rhs; return *this; }

#define VRT_MTH_DEFINE_OPERATOR_PACK_VEC2(Oper)           \
  VRT_MTH_DEFINE_OPERATOR_VEC2_VEC2(Oper)                 \
  VRT_MTH_DEFINE_ASSIGNMENT_OPERATOR_VEC2_VEC2(Oper)      \
  VRT_MTH_DEFINE_OPERATOR_VEC2_COMPONENT(Oper)            \
  VRT_MTH_DEFINE_ASSIGNMENT_OPERATOR_VEC2_COMPONENT(Oper) \

namespace vrt
{
  namespace mth
  {
    constexpr DOUBLE PI = 3.14159265357989;

    template <typename type>
      concept arithmetic = std::is_arithmetic_v<type>;

    template <arithmetic component>
      component DegreesToRadians( component D )
      {
        return D * static_cast<component>(PI / 180.0);
      } /* D2R */

    template <arithmetic component>
      component RadiansToDegrees( component R )
      {
        return R * static_cast<component>(180.0 / PI);
      } /* R2D */

    template <arithmetic component>
      class vec3
      {
      public:
        component X, Y, Z; // components

        vec3( VOID )
        {

        } /* vec3 */

        vec3( component X, component Y, component Z ) : X(X), Y(Y), Z(Z)
        {

        } /* vec3 */

        explicit vec3( component v ) : X(v), Y(v), Z(v)
        {

        } /* vec3 */

        component operator&( const vec3<component> &Rhs ) const
        {
          return X * Rhs.X + Y * Rhs.Y + Z * Rhs.Z;
        } /* operator& */

        vec3<component> operator%( const vec3<component> &Rhs ) const
        {
          return vec3<component>(
            Y * Rhs.Z - Z * Rhs.Y,
            Z * Rhs.X - X * Rhs.Z,
            X * Rhs.Y - Y * Rhs.X
          );
        } /* operator% */

        vec3<component> Normalize( VOID ) const
        {
          component length = std::sqrt(X * X + Y * Y + Z * Z);

          return vec3<component>(X / length, Y / length, Z / length);
        } /* Normalize */

        VRT_MTH_DEFINE_OPERATOR_PACK_VEC3(+)
        VRT_MTH_DEFINE_OPERATOR_PACK_VEC3(-)
        VRT_MTH_DEFINE_OPERATOR_PACK_VEC3(/)
        VRT_MTH_DEFINE_OPERATOR_PACK_VEC3(*)
      }; /* vec3 */

    template <arithmetic component>
      class vec2
      {
      public:
        component X, Y;

        vec2( VOID )
        {

        } /* vec3 */

        vec2( component X, component Y ) : X(X), Y(Y)
        {

        } /* vec3 */

        explicit vec2( component v ) : X(v), Y(v)
        {

        } /* vec3 */

        component operator&( const vec2<component> &Rhs )
        {
          return X * Rhs.X + Y * Rhs.Y;
        } /* operator& */

        component operator%( const vec2<component> &Rhs )
        {
          return X * Rhs.Y - Y * Rhs.X;
        } /* operator% */

        vec2<component> & Normalize( VOID ) const
        {
          component length = std::sqrt(X * X + Y * Y);

          return vec3(X / length, Y / length);
        } /* Normalize */

        VRT_MTH_DEFINE_OPERATOR_PACK_VEC2(+)
        VRT_MTH_DEFINE_OPERATOR_PACK_VEC2(-)
        VRT_MTH_DEFINE_OPERATOR_PACK_VEC2(/)
        VRT_MTH_DEFINE_OPERATOR_PACK_VEC2(*)
      }; /* vec2 */

    template <arithmetic component>
      class mat3
      {
      public:
        component Data[3][3]; // mat3
      }; /* mat */

    template <arithmetic component>
      class mat4
      {
      public:
        component Data[4][4];

        mat4( VOID ) : Data {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
          }
        {

        } /* mat4 */

        mat4(
          component d00, component d01, component d02, component d03,
          component d10, component d11, component d12, component d13,
          component d20, component d21, component d22, component d23,
          component d30, component d31, component d32, component d33
        ) : Data {
            {d00, d01, d02, d03},
            {d10, d11, d12, d13},
            {d20, d21, d22, d23},
            {d30, d31, d32, d33}
          }
        {

        } /* mat4 */

        inline operator component*( VOID )
        {
          return *Data;
        } /* operator component* */

        mat4 Transpose( VOID ) const
        {
          return mat4<component>(
            Data[0][0], Data[1][0], Data[2][0], Data[3][0],
            Data[0][1], Data[1][1], Data[2][1], Data[3][1],
            Data[0][2], Data[1][2], Data[2][2], Data[3][2],
            Data[0][3], Data[1][3], Data[2][3], Data[3][3]
          );
        } /* Transpose */

        mat4 operator*( const mat4 &rhs ) const
        {
          return mat4(
            Data[0][0] * rhs.Data[0][0] + Data[0][1] * rhs.Data[1][0] + Data[0][2] * rhs.Data[2][0] + Data[0][3] * rhs.Data[3][0],
            Data[0][0] * rhs.Data[0][1] + Data[0][1] * rhs.Data[1][1] + Data[0][2] * rhs.Data[2][1] + Data[0][3] * rhs.Data[3][1],
            Data[0][0] * rhs.Data[0][2] + Data[0][1] * rhs.Data[1][2] + Data[0][2] * rhs.Data[2][2] + Data[0][3] * rhs.Data[3][2],
            Data[0][0] * rhs.Data[0][3] + Data[0][1] * rhs.Data[1][3] + Data[0][2] * rhs.Data[2][3] + Data[0][3] * rhs.Data[3][3],

            Data[1][0] * rhs.Data[0][0] + Data[1][1] * rhs.Data[1][0] + Data[1][2] * rhs.Data[2][0] + Data[1][3] * rhs.Data[3][0],
            Data[1][0] * rhs.Data[0][1] + Data[1][1] * rhs.Data[1][1] + Data[1][2] * rhs.Data[2][1] + Data[1][3] * rhs.Data[3][1],
            Data[1][0] * rhs.Data[0][2] + Data[1][1] * rhs.Data[1][2] + Data[1][2] * rhs.Data[2][2] + Data[1][3] * rhs.Data[3][2],
            Data[1][0] * rhs.Data[0][3] + Data[1][1] * rhs.Data[1][3] + Data[1][2] * rhs.Data[2][3] + Data[1][3] * rhs.Data[3][3],

            Data[2][0] * rhs.Data[0][0] + Data[2][1] * rhs.Data[1][0] + Data[2][2] * rhs.Data[2][0] + Data[2][3] * rhs.Data[3][0],
            Data[2][0] * rhs.Data[0][1] + Data[2][1] * rhs.Data[1][1] + Data[2][2] * rhs.Data[2][1] + Data[2][3] * rhs.Data[3][1],
            Data[2][0] * rhs.Data[0][2] + Data[2][1] * rhs.Data[1][2] + Data[2][2] * rhs.Data[2][2] + Data[2][3] * rhs.Data[3][2],
            Data[2][0] * rhs.Data[0][3] + Data[2][1] * rhs.Data[1][3] + Data[2][2] * rhs.Data[2][3] + Data[2][3] * rhs.Data[3][3],

            Data[3][0] * rhs.Data[0][0] + Data[3][1] * rhs.Data[1][0] + Data[3][2] * rhs.Data[2][0] + Data[3][3] * rhs.Data[3][0],
            Data[3][0] * rhs.Data[0][1] + Data[3][1] * rhs.Data[1][1] + Data[3][2] * rhs.Data[2][1] + Data[3][3] * rhs.Data[3][1],
            Data[3][0] * rhs.Data[0][2] + Data[3][1] * rhs.Data[1][2] + Data[3][2] * rhs.Data[2][2] + Data[3][3] * rhs.Data[3][2],
            Data[3][0] * rhs.Data[0][3] + Data[3][1] * rhs.Data[1][3] + Data[3][2] * rhs.Data[2][3] + Data[3][3] * rhs.Data[3][3]
          );
        } /* operator* */


        /* 3x3 matrix determinator function.
         * ARGUMENTS:
         *   - matrix data:
         *       component A00, A01, A02,
         *                 A10, A11, A12,
         *                 A20, A21, A22;
         * RETURNS:
         *   (component) this matrix determinator.
         */
        static component MatrDeterm3x3( component A00, component A01, component A02,
                                        component A10, component A11, component A12,
                                        component A20, component A21, component A22 )
        {
          return A00 * A11 * A22 + A01 * A12 * A20 + A02 * A10 * A21 -
                 A00 * A12 * A21 - A01 * A10 * A22 - A02 * A11 * A20;
        } /* End of 'MatrDeterm3x3' function. */

        /* mat4 determinator operator override function.
         * ARGUMENTS: None.
         * RETURNS:
         *   (component) Operator result.
         */
        component operator!( VOID ) const
        {
          return
            +Data[0][0] * MatrDeterm3x3(Data[1][1], Data[1][2], Data[1][3],
                                        Data[2][1], Data[2][2], Data[2][3],
                                        Data[3][1], Data[3][2], Data[3][3]) +

            -Data[0][1] * MatrDeterm3x3(Data[1][0], Data[1][2], Data[1][3],
                                        Data[2][0], Data[2][2], Data[2][3],
                                        Data[3][0], Data[3][2], Data[3][3]) +

            +Data[0][2] * MatrDeterm3x3(Data[1][0], Data[1][1], Data[1][3],
                                        Data[2][0], Data[2][1], Data[2][3],
                                        Data[3][0], Data[3][1], Data[3][3]) +

            -Data[0][3] * MatrDeterm3x3(Data[1][0], Data[1][1], Data[1][2],
                                        Data[2][0], Data[2][1], Data[2][2],
                                        Data[3][0], Data[3][1], Data[3][2]);
        } /* End of 'operator!' function */

        /* Inversed matrix function.
         * ARGUMENTS: None.
         * RETURNS:
         *   (component) Inversed matrix.
         */
        mat4 Inverse( VOID ) const
        {
          component det = !*this;
          mat4 r;

          if (det == 0)
            return mat4();

          r.Data[0][0] =
            +MatrDeterm3x3(Data[1][1], Data[1][2], Data[1][3],
                           Data[2][1], Data[2][2], Data[2][3],
                           Data[3][1], Data[3][2], Data[3][3]) / det;
          r.Data[1][0] =
            -MatrDeterm3x3(Data[1][0], Data[1][2], Data[1][3],
                           Data[2][0], Data[2][2], Data[2][3],
                           Data[3][0], Data[3][2], Data[3][3]) / det;
          r.Data[2][0] =
            +MatrDeterm3x3(Data[1][0], Data[1][1], Data[1][3],
                           Data[2][0], Data[2][1], Data[2][3],
                           Data[3][0], Data[3][1], Data[3][3]) / det;
          r.Data[3][0] =
            -MatrDeterm3x3(Data[1][0], Data[1][1], Data[1][2],
                           Data[2][0], Data[2][1], Data[2][2],
                           Data[3][0], Data[3][1], Data[3][2]) / det;

          r.Data[0][1] =
            -MatrDeterm3x3(Data[0][1], Data[0][2], Data[0][3],
                           Data[2][1], Data[2][2], Data[2][3],
                           Data[3][1], Data[3][2], Data[3][3]) / det;
          r.Data[1][1] =
            +MatrDeterm3x3(Data[0][0], Data[0][2], Data[0][3],
                           Data[2][0], Data[2][2], Data[2][3],
                           Data[3][0], Data[3][2], Data[3][3]) / det;
          r.Data[2][1] =
            -MatrDeterm3x3(Data[0][0], Data[0][1], Data[0][3],
                           Data[2][0], Data[2][1], Data[2][3],
                           Data[3][0], Data[3][1], Data[3][3]) / det;
          r.Data[3][1] =
            +MatrDeterm3x3(Data[0][0], Data[0][1], Data[0][2],
                           Data[2][0], Data[2][1], Data[2][2],
                           Data[3][0], Data[3][1], Data[3][2]) / det;

          r.Data[0][2] =
            +MatrDeterm3x3(Data[0][1], Data[0][2], Data[0][3],
                           Data[1][1], Data[1][2], Data[1][3],
                           Data[3][1], Data[3][2], Data[3][3]) / det;
          r.Data[1][2] =
            -MatrDeterm3x3(Data[0][0], Data[0][2], Data[0][3],
                           Data[3][0], Data[3][2], Data[3][3],
                           Data[1][0], Data[1][2], Data[1][3]) / det;
          r.Data[2][2] =
            +MatrDeterm3x3(Data[0][0], Data[0][1], Data[0][3],
                           Data[1][0], Data[1][1], Data[1][3],
                           Data[3][0], Data[3][1], Data[3][3]) / det;
          r.Data[3][2] =
            -MatrDeterm3x3(Data[0][0], Data[0][1], Data[0][2],
                           Data[1][0], Data[1][1], Data[1][2],
                           Data[3][0], Data[3][1], Data[3][2]) / det;

          r.Data[0][3] =
            -MatrDeterm3x3(Data[0][1], Data[0][2], Data[0][3],
                           Data[1][1], Data[1][2], Data[1][3],
                           Data[2][1], Data[2][2], Data[2][3]) / det;
          r.Data[1][3] =
            +MatrDeterm3x3(Data[0][0], Data[0][2], Data[0][3],
                           Data[1][0], Data[1][2], Data[1][3],
                           Data[2][0], Data[2][2], Data[2][3]) / det;
          r.Data[2][3] =
            -MatrDeterm3x3(Data[0][0], Data[0][1], Data[0][3],
                           Data[1][0], Data[1][1], Data[1][3],
                           Data[2][0], Data[2][1], Data[2][3]) / det;
          r.Data[3][3] =
            +MatrDeterm3x3(Data[0][0], Data[0][1], Data[0][2],
                           Data[1][0], Data[1][1], Data[1][2],
                           Data[2][0], Data[2][1], Data[2][2]) / det;

          return r;
        } /* End of 'Inverse' function */

        /* Rotate matrix function.
         * ARGUMENTS:
         *   - angle to rotate by:
         *       const component &Angle;
         *   - vector to rotate by:
         *       const vec3<component> &v;
         * RETURNS:
         *   (mat4) Rotate matrix;
         */
        static mat4 Rotate( component Angle, const vec3<component> &V )
        {
          vec3<component> VN = V.Normalize();

          component
            s = std::sin(Angle),
            c = std::cos(Angle);

          return mat4(
            VN.X * VN.X * (1 - c) + c,          VN.X * VN.Y * (1 - c) - VN.Z * s,   VN.X * VN.Z * (1 - c) + VN.Y * s,   0,
            VN.Y * VN.X * (1 - c) + VN.Z * s,   VN.Y * VN.Y * (1 - c) + c,          VN.Y * VN.Z * (1 - c) - VN.X * s,   0,
            VN.Z * VN.X * (1 - c) - VN.Y * s,   VN.Z * VN.Y * (1 - c) + VN.X * s,   VN.Z * VN.Z * (1 - c) + c,          0,
            0,                                  0,                                  0,                                  1);
        } /* End of 'Rotate' function. */

        /* Matrix rotation by X coord to Angle getting function.
         * ARGUMETNS:
         *   - angle;
         *       const component &Angle;
         * RETURNS:
         *   (mat4) Rotate matrix.
         */
        static mat4 RotateX( const component &Angle )
        {
          component
            s = std::sin(Angle),
            c = std::cos(Angle);

          return mat4(1,  0, 0, 0,
                      0,  c, s, 0,
                      0, -s, c, 0,
                      0,  0, 0, 1);
        } /* End of 'RotateX' function. */

        /* Matrix rotation by Y coord to Angle getting function.
         * ARGUMETNS:
         *   - angle;
         *       const component &Angle;
         * RETURNS:
         *   (mat4) Rotate matrix.
         */
        static mat4 RotateY( const component &Angle )
        {
          component
            s = std::sin(Angle),
            c = std::cos(Angle);

          return mat4(c, 0, -s, 0,
                      0, 1,  0, 0,
                      s, 0,  c, 0,
                      0, 0,  0, 1);
        } /* End of 'RotateY' function. */

        /* Matrix rotation by Z coord to Angle function.
         * ARGUMETNS:
         *   - angle;
         *       const component &Angle;
         * RETURNS:
         *   (mat4) Rotate matrix.
         */
        static mat4 RotateZ( const component &Angle )
        {
          component
            s = std::sin(Angle),
            c = std::cos(Angle);

          return mat4( c, s, 0, 0,
                      -s, c, 0, 0,
                       0, 0, 1, 0,
                       0, 0, 0, 1);
        } /* End of 'RotateZ' function. */

        /* Matrix scaling function.
         * ARGUMENTS:
         *   - vector to scale by:
         *       const vec3<component> &v;
         * RETURNS:
         *   (mat4) scale matrix.
         */
        static mat4 Scale( const vec3<component> &v )
        {
          return mat4(v.X,   0,   0, 0,
                        0, v.Y,   0, 0,
                        0,   0, v.Z, 0,
                        0,   0,   0, 1);
        } /* End of 'Scale' function */

        /* Matrix translation function.
         * ARGUMENTS:
         *   - vector to translate by:
         *       const vec3<component> &v;
         * RETURNS:
         *   (mat4) translate vector.
         */
        static mat4 Translate( const vec3<component> &v )
        {
          return mat4(  1,   0,   0, 0,
                        0,   1,   0, 0,
                        0,   0,   1, 0,
                      v.X, v.Y, v.Z, 1);
        } /* End of 'Translate' function */

        /* Vector transform function.
         * ARGUMENTS:
         *   - vector to transform:
         *       const vec3<component> &v;
         * RETURNS:
         *   (vec3<component>) Transformed vector.
         */
        vec3<component> TransformVector( const vec3<component> &v ) const //V * mat3x3
        {
          return vec3<component>(v.X * Data[0][0] + v.Y * Data[1][0] + v.Z * Data[2][0],
                                 v.X * Data[0][1] + v.Y * Data[1][1] + v.Z * Data[2][1],
                                 v.X * Data[0][2] + v.Y * Data[1][2] + v.Z * Data[2][2]);
        } /* End of 'TransformVector' function. */

        /* Point transform function.
         * ARGUMENTS:
         *   - vector to transform:
         *       const vec3<component> &v;
         * RETURNS:
         *   (vec3<component>) Transformed vector.
         */
        vec3<component> TransformPoint( const vec3<component> &v ) const //V * mat4x3
        {
          return vec3<component>(v.X * Data[0][0] + v.Y * Data[1][0] + v.Z * Data[2][0] + Data[3][0],
                                 v.X * Data[0][1] + v.Y * Data[1][1] + v.Z * Data[2][1] + Data[3][1],
                                 v.X * Data[0][2] + v.Y * Data[1][2] + v.Z * Data[2][2] + Data[3][2]);
        } /* End of 'TransformPoint' function. */

        /* Vector transform function.
         * ARGUMENTS:
         *   - vector to transform:
         *       const vec3<component> &v;
         * RETURNS:
         *   (vec3<component>) Transformed vector.
         */
        vec3<component> Transform4x4( const vec3<component> &v ) const //V * mat4x4
        {
          component w = v.X * Data[0][3] + v.Y * Data[1][3] + v.Z * Data[2][3] + Data[3][3];

          return vec3<component>((v.X * Data[0][0] + v.Y * Data[1][0] + v.Z * Data[2][0] + Data[3][0]) / w,
                                 (v.X * Data[0][1] + v.Y * Data[1][1] + v.Z * Data[2][1] + Data[3][1]) / w,
                                 (v.X * Data[0][2] + v.Y * Data[1][2] + v.Z * Data[2][2] + Data[3][2]) / w);
        } /* End of 'Transform4x4' function. */

        /* Vector transform function.
         * ARGUMENTS:
         *   - vector to transform:
         *       const vec3<component> &v;
         * RETURNS:
         *   (vec3<component>) Transformed vector.
         */
        vec3<component> TransformNormal( const vec3<component> &v ) const
        {
          mat4<component> t = Inverse();
          return vec3<component>(v.X * t.Data[0][0] + v.Y * t.Data[0][1] + v.Z * t.Data[0][2],
                                 v.X * t.Data[1][0] + v.Y * t.Data[1][1] + v.Z * t.Data[1][2],
                                 v.X * t.Data[2][0] + v.Y * t.Data[2][1] + v.Z * t.Data[2][2]);
        } /* End of 'TransformVector' function. */

        vec3<component> operator*( const vec3<component> &V ) const
        {
          return Transform4x4(V);
        } /* operator* */

        /* Orthographical projection matrixix
         * ARGUMENTS:
         *   - options of Orthographical projection
         *       component L, R, B, T, N, F;
         * RETURNS:
         *   (mat4) Orthographical matrix.
         */
        static mat4 Ortho( component L, component R, component B, component T, component N, component F )
        {
          return mat4(
            2 / (R - L),        0,                  0,                  0,
            0,                  -2 / (T - B),       0,                  0,
            0,                  0,                  -2 / (F - N),       0,
            -(R + L) / (R - L), -(T + B) / (T - B), -(F + N) / (F - N), 1
            );
        } /* End of 'Ortho' function */

        /* Frustum projection matrix
         * ARGUMENTS:
         *   - options of Frustum projection
         *       component L, R, B, T, N, F;
         * RETURNS:
         *   (mat4) Frustum matrix.
         */
        static mat4 Frustum( component L, component R, component B, component T, component N, component F )
        {
          return mat4(
            2 * N / (R - L),   0,                 0,                    0,
            0,                 -2 * N / (T - B),  0,                    0,
            (R + L) / (R - L), (T + B) / (T - B), -(F + N) / (F - N),  -1,
            0,                 0,                 -2 * N * F / (F - N), 0);
        } /* End of 'Frustum' function */

        /* View matrix function
         * ARGUMENTS:
         *   - position of viewer
         *       const vec3<component> &Loc;
         *   - point viewer looking at
         *       const vec3<component> &At;
         *   - vector of viewer`s head
         *       const vec3<component> &Up;
         * RETURNS:
         *   (mat4) View matrix.
         */
        static mat4 View( const vec3<component> &Location, const vec3<component> &At, const vec3<component> &Up )
        {
          vec3<component>
            Direction = (At - Location).Normalize(),
            Right = (Direction % Up).Normalize(),
            TUp = (Right % Direction).Normalize();

          return mat4(
            Right.X,                TUp.X,               -Direction.X,               0,
            Right.Y,                TUp.Y,               -Direction.Y,               0,
            Right.Z,                TUp.Z,               -Direction.Z,               0,
            -(Location & Right),         -(Location & TUp),        (Location & Direction),          1);
        } /* End of 'View' function */
      }; /* mat4 */

    template <arithmetic component>
      class camera
      {
      public:
        vec3<component>
          Location  = vec3<component>(0, 0, 0), // Camera position
          Up        = vec3<component>(0, 1, 0), // Camera Up vector
          Right     = vec3<component>(1, 0, 0), // Camera Right vector
          Direction = vec3<component>(0, 0, 1), // Camera direction vector
          At        = vec3<component>(0, 0, 5); // Camera at vector
        component
          FrameW   = 320.0f,                  // Frame width
          FrameH   = 240.0f,                  // Frame height
          ProjectionSize = 0.2f,              // Projection size
          NearPlane     = 0.1f,               // Near plane
          FarPlane      = 16384.0f;           // Far plane
        mat4<component>
          Projection,                        // Projection matrix
          View,                              // View matrix
          ViewProjection;                    // View * Projection matrix

        //default camera constructor
        camera( VOID )
        {

        } /* End of 'camera' function */

        /* Camera constructor function.
         * ARGUMENTS:
         *   - loc vector:
         *       const vec3 &L;
         *   - up vector:
         *       const vec3 &U;
         *   - dir vector:
         *       const vec3 &D;
         */
        camera( const vec3<component> &L, const vec3<component> &D, const vec3<component> &U = vec3<component>(0, 1, 0) ) : Location(L), Up(U), Direction(D), Right((U % D).Normalize())
        {
        } /* End of 'camera' function. */

        /* Reshape camera function.
         * ARGUMENTS:
         *   - new frame size:
         *       FLOAT NewFW, NewFH;
         * RETURNS: None.
         */
        VOID Reshape( component NewFW, component NewFH )
        {
          FrameW = NewFW;
          FrameH = NewFH;
          ResetProjection();
        } /* End of 'Reshape' function */

        /* Projection setting function.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        VOID ResetProjection( VOID )
        {
          component
            rx = ProjectionSize,
            ry = ProjectionSize;

          if (FrameW > FrameH)
            rx *= (component)FrameW / FrameH;
          else
            ry *= (component)FrameH / FrameW;

          Projection = mat4<component>::Frustum(-rx / 2, rx / 2, -ry / 2, ry / 2, NearPlane, FarPlane);
          ViewProjection = View * Projection;
        } /* End of 'ProjSet' function */

        /* Ortho matrix projection function.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        VOID ResetProjectionOrtho( VOID )
        {
          component
            rx = ProjectionSize,
            ry = ProjectionSize;

          if (FrameW > FrameH)
            rx *= (component)FrameW / FrameH;
          else
            ry *= (component)FrameH / FrameW;

          Projection = mat4<component>::Ortho(-rx / 2, rx / 2, -ry / 2, ry / 2, NearPlane, FarPlane);
          ViewProjection = View * Projection;
        } /* End of 'ProjSetOrtho' function */

        /* Camera setting function.
         * ARGUMENTS:
         *   - camera location:
         *       const vec3 &nLoc;
         *   - point of view:
         *       const vec3 &nAt;
         *   - up vector:
         *       const vec3 &nUp;
         * RETURNS: None.
         */
        VOID Set( const vec3<component> &nLoc, const vec3<component> &nAt, const vec3<component> &nUp = vec3<component>(0, 1, 0) )
        {
          View = mat4<component>::View(nLoc, nAt, nUp);
          Right = vec3<component>( View.Data[0][0],
                                   View.Data[1][0],
                                   View.Data[2][0]);
          Up =    vec3<component>( View.Data[0][1],
                                   View.Data[1][1],
                                   View.Data[2][1]);
          Direction =   vec3<component>(-View.Data[0][2],
                                  -View.Data[1][2],
                                  -View.Data[2][2]);
          Location = nLoc;
          At = nAt;
          ViewProjection = View * Projection;
        } /* End of 'Set' function */
      }; /* camera */
  } /* mth */

  using ivec2 = mth::vec2<INT>;
  using ivec3 = mth::vec3<INT>;
  using imat4 = mth::mat4<INT>;
  using imat3 = mth::mat3<INT>;

  using vec2 = mth::vec2<FLOAT>;
  using vec3 = mth::vec3<FLOAT>;
  using mat4 = mth::mat4<FLOAT>;
  using mat3 = mth::mat3<FLOAT>;

  using dvec2 = mth::vec2<DOUBLE>;
  using dvec3 = mth::vec3<DOUBLE>;
  using dmat4 = mth::mat4<DOUBLE>;
  using dmat3 = mth::mat3<DOUBLE>;

  using camera = mth::camera<FLOAT>;
} /* namespace vrt */

#endif // !defined __vrt_mth_h_