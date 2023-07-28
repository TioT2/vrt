#ifndef __vrt_utils_h_
#define __vrt_utils_h_

#include "vrt_def.h"

namespace vrt
{
  namespace utils
  {
    namespace file
    {
      inline std::vector<BYTE> LoadBinary( std::string_view filePath )
      {
        std::FILE *file = std::fopen(filePath.data(), "rb");

        if (file == nullptr)
          return std::vector<BYTE>();

        std::fseek(file, 0, SEEK_END);
        std::vector<BYTE> fileData;
        fileData.resize(std::ftell(file));
        std::fseek(file, 0, SEEK_SET);

        std::fread(fileData.data(), 1, fileData.size(), file);

        std::fclose(file);

        return fileData;
      } /* loadBinary */

      inline std::string LoadText( std::string_view filePath )
      {
        std::FILE *file = std::fopen(filePath.data(), "r");

        if (file == nullptr)
          throw std::runtime_error(std::format("can't open file \"{}\"", filePath));

        std::fseek(file, 0, SEEK_END);
        std::string fileData;
        fileData.resize(std::ftell(file));
        std::fseek(file, 0, SEEK_SET);

        std::fread(fileData.data(), 1, fileData.size(), file);

        std::fclose(file);

        return fileData;
      } /* loadText */
    } /* namespace file */

    struct image
    {
      // image has missing texture colors by default
      SIZE_T Width = 2, Height = 2;
      std::vector<UINT32> Pixels
      {
        0xFFFF00FF,
        0xFF000000,
        0xFF000000,
        0xFFFF00FF
      };

      SIZE_T SizeBytes( VOID ) const
      {
        return Width * Height * sizeof(UINT32);
      } /* SizeBytes */

      BOOL Load( std::string_view path )
      {
        SDL_Surface *unformattedImage = IMG_Load(path.data());

        if (unformattedImage == nullptr)
          return FALSE;

        SDL_Surface *image = SDL_ConvertSurfaceFormat(unformattedImage, SDL_PIXELFORMAT_ABGR8888, 0);

        SDL_LockSurface(image);

        Width = image->w;
        Height = image->h;

        Pixels.resize(Width * Height);
        for (INT y = 0; y < image->h; y++)
          std::memcpy(&Pixels[y * Width], (BYTE *)image->pixels + image->pitch * y, Width * sizeof(UINT32));

        SDL_UnlockSurface(image);

        SDL_FreeSurface(image);
        SDL_FreeSurface(unformattedImage);

        return TRUE;
      } /* Load */

      BOOL Save( std::string_view path )
      {
        if (Width == 0 || Height == 0)
          return FALSE;

        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(Pixels.data(), static_cast<INT>(Width), static_cast<INT>(Height), 1, static_cast<INT>(Width * sizeof(UINT32)), 0, 0, 0, 0);

        if (surface == nullptr)
          return FALSE;

        BOOL result = (IMG_SavePNG(surface, std::format("{}.png", path.substr(0, path.find_last_of('.'))).c_str()) == 0);

        SDL_FreeSurface(surface);

        return result;
      } /* Save */
    }; /* class image */

    class timer
    {
      std::chrono::high_resolution_clock::time_point InitialTime, CurrentTime;
      UINT32 TotalFrameCount = 0;

      BOOL Paused = FALSE;
      std::chrono::high_resolution_clock::time_point LastPauseStart;
      std::chrono::high_resolution_clock::duration PauseTimeCollector;

      FLOAT GlobalDeltaTime = 0.0f;
      FLOAT GlobalTime = 0.0f;

      FLOAT Time = 0.0f;
      FLOAT DeltaTime = 0.0f;


      UINT32 FPS_FrameCounter = 0;
      std::chrono::high_resolution_clock::time_point FPS_LastUpdateTime;
      FLOAT FPS = NAN;
      FLOAT FPS_TimeAverage = NAN;

    public:
      FLOAT FPS_UpdateDuration = 3.0f;


      timer( VOID )
      {
        InitialTime = std::chrono::high_resolution_clock::now();
        FPS_LastUpdateTime = InitialTime;
        PauseTimeCollector = std::chrono::high_resolution_clock::duration::min(); // minimal duration
      }  /* Timer */


      BOOL SetPause( BOOL newPaused )
      {
        if (Paused == newPaused)
          return FALSE;

        if (Paused)
          PauseTimeCollector += std::chrono::high_resolution_clock::now() - LastPauseStart;
        else
          LastPauseStart = std::chrono::high_resolution_clock::now();

        Paused = newPaused;
      } /* setPause */


      VOID SwitchPause( VOID )
      {
        SetPause(!Paused);
      } /* switchPause */


      FLOAT GetCurrentTime( VOID )
      {
        return std::chrono::duration<FLOAT>(std::chrono::high_resolution_clock::now() - InitialTime).count();
      } /* getCurrentTime */


      VOID Response( VOID )
      {
        CurrentTime = std::chrono::high_resolution_clock::now();
        TotalFrameCount++;

        FLOAT oldGlobalTime = GlobalTime;
        GlobalTime = std::chrono::duration<FLOAT>(CurrentTime - InitialTime).count();
        GlobalDeltaTime = GlobalTime - oldGlobalTime;

        if (Paused)
        {
          DeltaTime = 0;
        }
        else
        {
          Time = std::chrono::duration<FLOAT>(CurrentTime - InitialTime).count();
          DeltaTime = GlobalDeltaTime;
        }

        // update FPS
        FPS_FrameCounter++;
        FLOAT fpsLastUpdateElapsedTime = std::chrono::duration<FLOAT>(CurrentTime - FPS_LastUpdateTime).count();
        if (fpsLastUpdateElapsedTime > FPS_UpdateDuration)
        {
          FPS = FPS_FrameCounter / fpsLastUpdateElapsedTime;
          FPS_LastUpdateTime = CurrentTime;
          FPS_FrameCounter = 0;
        }
        FPS_TimeAverage = TotalFrameCount / GlobalTime; // get average framerate
      } /* response */


      inline FLOAT GetTime( VOID )            const { return Time;            } /* getTime */
      inline FLOAT GetDeltaTime( VOID )       const { return DeltaTime;       } /* getDeltaTime */
      inline FLOAT GetGlobalTime( VOID )      const { return GlobalTime;      } /* getGlobalTime */
      inline FLOAT GetGlobalDeltaTime( VOID ) const { return GlobalDeltaTime; } /* getGlobalDeltaTime */

      inline BOOL IsPaused( VOID ) const { return Paused; } /* isPaused */
    }; /* timer */


    class input
    {
    public:
      constexpr static UINT KEY_NUMBER = 256;

      struct key_state
      {
        BYTE Pressed  : 1;
        BYTE Clicked  : 1;
        BYTE Released : 1;
        BYTE Old      : 1;
      }; /* KeyState */

    private:
      key_state KeyStates[KEY_NUMBER];
    public:

      VOID Response( VOID )
      {
        INT readKeyNumber = 0;

        BYTE temp[KEY_NUMBER]; // temporary array in case there is no new key data
        const BYTE *keys = SDL_GetKeyboardState(&readKeyNumber);

        if (readKeyNumber < KEY_NUMBER)
        {
          std::cerr << "input: can't read enough keys, no input recieved. previous frame input used.\n";

          // use previous frame state as current
          for (UINT i = 0; i < KEY_NUMBER; i++)
            temp[i] = KeyStates[i].Pressed;
          keys = temp; // assign temporary array as new
        }

        // update key states
        for (UINT i = 0; i < KEY_NUMBER; i++)
        {
          KeyStates[i].Old = KeyStates[i].Pressed;
          KeyStates[i].Pressed = keys[i];

          KeyStates[i].Clicked = (KeyStates[i].Pressed & !KeyStates[i].Old);
          KeyStates[i].Released = (!KeyStates[i].Pressed & KeyStates[i].Old);
        }
      } /* response */

      key_state GetKeyState( SDL_Scancode Scancode )
      {
        if ((INT)Scancode >= 256 || (INT)Scancode < 0)
          return key_state(0);
        return KeyStates[(INT)Scancode];
      } /* getKeyState */

      BOOL IsKeyPressed( SDL_Scancode Scancode )
      {
        if ((INT)Scancode >= 256 || (INT)Scancode < 0)
          return FALSE;
        return KeyStates[(INT)Scancode].Pressed;
      } /* isKeyPressed */

      BOOL IsKeyOldPressed( SDL_Scancode Scancode )
      {
        if ((INT)Scancode >= 256 || (INT)Scancode < 0)
          return FALSE;
        return KeyStates[(INT)Scancode].Old;
      } /* isKeyOldPressed */

      BOOL IsKeyClicked( SDL_Scancode Scancode )
      {
        if ((INT)Scancode >= 256 || (INT)Scancode < 0)
          return FALSE;
        return KeyStates[(INT)Scancode].Clicked;
      } /* isKeyClicked */

      BOOL IsKeyReleased( SDL_Scancode Scancode )
      {
        if ((INT)Scancode >= 256 || (INT)Scancode < 0)
          return FALSE;
        return KeyStates[(INT)Scancode].Released;
      } /* isKeyReleased */
    }; /* Input */

    class splitter
    {
      std::string_view Str = "";
      CHAR Symbol = 0;
      std::size_t Begin = 0;
      std::size_t End = 0;
    public:

      splitter( VOID )
      {

      } /* splitter */

      splitter( std::string_view Str, CHAR Symbol ) : Str(Str), Symbol(Symbol)
      {
        Begin = 0;
        End = Str.find(Symbol, 0);
      } /* splitter */

      std::string_view Get( VOID )
      {
        std::string_view View;

        if (Begin != std::string::npos)
        {
          View = {&Str[Begin], &Str[End]};

          Begin = End + 1;
          End = Str.find(Symbol, Begin);

          if (End == std::string::npos)
            End = View.length();
        }

        return View;
      } /* Get */
    }; /* splitter */

    inline SIZE_T Align( SIZE_T Value, SIZE_T Alignment )
    {
      SIZE_T Tail = Value % Alignment;

      return Tail == 0 ? Value : Value - Tail + Alignment;
    } /* Align */

    inline BOOL CheckFlags( UINT32 Where, UINT32 Flags )
    {
      return (Where & Flags) == Flags;
    } /* checkFlags */

    inline VOID AssertResult( VkResult Result, std::string_view Message = "" )
    {
      if (Result != VK_SUCCESS)
      {
        std::string ErrorMessage = std::format("Error: {}. {}", string_VkResult(Result), Message);

        std::cout << ErrorMessage << "\n";
        throw std::runtime_error(ErrorMessage);
      }
    } /* AssertResult */

    inline VOID Assert( BOOL Expression, std::string_view Message )
    {
      if (!Expression)
      {
        std::string ErrorMessage = std::format("Error: {}", Message);

        std::cout << ErrorMessage << "\n";
        throw std::runtime_error(ErrorMessage);
      }
    } /* Assert */

    inline VOID NoImplementation( std::string_view Message = "No functionality implementation" )
    {
      throw std::logic_error(Message.data());
    } /* NoImplementation */
  } /* namespace utils */
} /* vrt */

#endif // !defined __vrt_utils_h_