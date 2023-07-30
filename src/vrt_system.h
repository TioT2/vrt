#ifndef __vrt_system_h_
#define __vrt_system_h_

#include "render/vrt_render.h"

#define VRT_UNIT_REGISTER(UnitName) static vrt::unit_register<UnitName> __register_##UnitName {#UnitName}
#define VRT_UNIT_REGISTER_MAIN(UnitName) static vrt::main_unit_register<UnitName> __main_register {#UnitName}


namespace vrt
{
  class scene;
  class system;

  class unit abstract
  {
  public:
    static std::string IdConstructionParameter;
    static scene *SceneConstructionParameter;

    scene *const Scene;
    const std::string Id;

    unit( VOID ) : Scene(SceneConstructionParameter), Id(IdConstructionParameter)
    {

    } /* unit */

    virtual VOID Response( system &system ) = 0;

    virtual ~unit( VOID )
    {

    } /* ~unit */
  }; /* unit */

  using unit_create_function = unit * (*)( system &, const std::string &, scene * );

  struct unit_register_table
  {
    static unit_register_table & GetTable( VOID )
    {
      static unit_register_table Instance;

      return Instance;
    } /* Get */

    std::map<std::string, unit_create_function> Table; // table

    static std::string MainId; // Main unit
  public:

    static VOID Register( std::string_view Id, unit_create_function Func )
    {
      GetTable().Table[std::string(Id)] = Func;
    } /* Register */

    static VOID RegisterMain( std::string_view Id, unit_create_function Func )
    {
      GetTable().MainId = Id;
    } /* RegisterMain */

    static unit_create_function Get( const std::string &Id )
    {
      auto Table = GetTable();
      auto iter = Table.Table.find(Id);

      if (iter == Table.Table.end())
        return nullptr;
      return iter->second;
    } /* Get */
  }; /* unit_register_table */

  template <typename unit_type> requires std::is_base_of_v<unit, unit_type>
    struct unit_register
    {
      static unit * Create( system &System, const std::string &Id, scene *Scene )
      {
        unit::IdConstructionParameter = Id;
        unit::SceneConstructionParameter = Scene;

        return static_cast<unit *>(new unit_type(System));
      } /* Create */

      unit_register( const std::string &Id )
      {
        unit_register_table::Register(Id, Create);
      } /* unit_register */
    }; /* unit_register */

  template <typename unit_type> requires std::is_base_of_v<unit, unit_type>
    struct main_unit_register : unit_register<unit_type>
    {
      main_unit_register( const std::string &Id ) : unit_register<unit_type>(Id)
      {
        unit_register_table::RegisterMain(Id, unit_register<unit_type>::Create);
      } /* main_unit_register */
    }; /* struct main_unit_register */

  class scene
  {
  public:
    system *System = nullptr;


    std::vector<unit *> Units; // Units
    std::string Name;          // Name

    scene( system *System ) : System(System)
    {

    } /* scene */

    VOID AddUnit( const std::string &Id )
    {
      if (auto Create = unit_register_table::Get(Id); Create != nullptr)
        Units.push_back(Create(*System, Id, this));
    } /* AddUnit */

    unit * FindUnit( std::string_view Id )
    {
      for (unit *Unit : Units)
        if (Unit->Id == Id)
          return Unit;
      return nullptr;
    } /* AddUnit */

    VOID Response( VOID )
    {
      for (unit *Unit : Units)
        Unit->Response(*System);
    } /* Response */

    VOID Clear( VOID )
    {
      for (unit *Unit : Units)
        delete Unit;
    } /* Clear */

    ~scene( VOID )
    {
      Clear();
    } /* ~scene */
  }; /* scene */

  class system
  {
    scene *Scene = nullptr;
    SDL_Window *Window = nullptr;
    struct { INT Width, Height; } OldWindowSize = {30, 30};

  public:
    std::span<const CHAR *> ConsoleArguments;
    BOOL IsFullscreen = FALSE;
    utils::timer Timer;
    utils::input Input;
    render::engine Render;

    VOID SwitchFullscreen( VOID )
    {
      if (IsFullscreen)
      {
        SDL_SetWindowFullscreen(Window, 0);
        SDL_SetWindowSize(Window, OldWindowSize.Width, OldWindowSize.Height);
      }
      else
      {
        SDL_GetWindowSize(Window, &OldWindowSize.Width, &OldWindowSize.Height);

        SDL_DisplayMode DisplayMode;

        SDL_GetCurrentDisplayMode(0, &DisplayMode);
        SDL_SetWindowSize(Window, DisplayMode.w, DisplayMode.h);
        SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN);
      }

      IsFullscreen = !IsFullscreen;
    } /* SwitchFullscreen */

    system( std::span<const CHAR *> ConsoleArguments = {} ) : ConsoleArguments(ConsoleArguments)
    {
      SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
      IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
      Window = SDL_CreateWindow("rtt", 30, 30, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

      Render.Initialize(Window);
    } /* system */

    VOID Run( VOID )
    {
      BOOL isRunning = TRUE;

      Scene = new scene(this);

      Scene->AddUnit(unit_register_table::MainId);

      while (isRunning)
      {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
          switch (event.type)
          {
          case SDL_QUIT:
            isRunning = FALSE;
            break;

          // handle default KEY_DOWN events
          case SDL_KEYDOWN:
            switch (event.key.keysym.scancode)
            {
            case SDL_SCANCODE_ESCAPE:
              isRunning = FALSE;
              break;

            case SDL_SCANCODE_F11:
              SwitchFullscreen();
              break;
            }
            break;

          case SDL_WINDOWEVENT:
            Render.Resize();
            break;
          }
        }

        Timer.Response();
        Input.Response();
        Scene->Response();
        Render.Render();
      }

      delete Scene;
    } /* Run */

    ~system( VOID )
    {
      Render.Close();
      SDL_DestroyWindow(Window);

      IMG_Quit();
      SDL_Quit();
    } /* ~system */
  }; /* class system */
} /* vrt */

#endif // !defined __vrt_system_h_