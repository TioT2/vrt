#include "vrt.h"

namespace vrt
{
  class camera_controller : public unit
  {
  public:
    camera_controller( system &System )
    {

    } /* camera_controller */

    VOID Response( system &System ) override
    {
      camera &Camera = System.Render.Scene->Camera;

      if (System.Input.IsKeyClicked(SDL_SCANCODE_SPACE))
        System.Render.DoPresentCollection = !System.Render.DoPresentCollection;

      vec3 MoveAxis
      {
        (FLOAT)(System.Input.IsKeyPressed(SDL_SCANCODE_D) - System.Input.IsKeyPressed(SDL_SCANCODE_A)),
        (FLOAT)(System.Input.IsKeyPressed(SDL_SCANCODE_R) - System.Input.IsKeyPressed(SDL_SCANCODE_F)),
        (FLOAT)(System.Input.IsKeyPressed(SDL_SCANCODE_W) - System.Input.IsKeyPressed(SDL_SCANCODE_S)),
      };
      vec2 RotateAxis
      {
        (FLOAT)(System.Input.IsKeyPressed(SDL_SCANCODE_RIGHT) - System.Input.IsKeyPressed(SDL_SCANCODE_LEFT)),
        (FLOAT)(System.Input.IsKeyPressed(SDL_SCANCODE_DOWN) - System.Input.IsKeyPressed(SDL_SCANCODE_UP)),
      };

      vec3 MovementDelta = (
          Camera.Right     * MoveAxis.X +
          Camera.Up        * MoveAxis.Y +
          Camera.Direction * MoveAxis.Z
        ) * System.Timer.GetDeltaTime() * 3.0;

      FLOAT Azimuth = std::acos(Camera.Direction.Y);
      FLOAT Elevator =
        Camera.Direction.Z / std::abs(Camera.Direction.Z) *
        std::acos(
          Camera.Direction.X /
          std::sqrt(
            Camera.Direction.X * Camera.Direction.X + Camera.Direction.Z * Camera.Direction.Z
          )
        );

      Elevator += RotateAxis.X * System.Timer.GetDeltaTime() * 2.0f;
      Azimuth += RotateAxis.Y * System.Timer.GetDeltaTime() * 2.0f;

      Azimuth = std::clamp<FLOAT>(Azimuth, 0.01f, static_cast<FLOAT>(mth::PI) - 0.01f);

      vec3 NewDirection
      {
        std::sin(Azimuth) * std::cos(Elevator),
        std::cos(Azimuth),
        std::sin(Azimuth) * std::sin(Elevator)
      };
      
      Camera.Set(Camera.Location + MovementDelta, NewDirection);
    } /* Response */
  }; /* camera_controller */

  VRT_UNIT_REGISTER(camera_controller);

  class main_unit : public unit
  {
  public:
    main_unit( system &System )
    {
      Scene->AddUnit("camera_controller");
    } /* main_unit */

    VOID Response( system &System ) override
    {

    } /* Response */
  }; /* main_unit */

  VRT_UNIT_REGISTER_MAIN(main_unit);
} /* vrt */