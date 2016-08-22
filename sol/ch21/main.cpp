#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif
#include "gp.h"

// Screen surface
SDL_Surface *gScreen;
// Surface that contains the tiles
SDL_Surface *gTiles;
// Surface that contains the font
SDL_Surface *gFont;

// Pointer to the joystick interface
SDL_Joystick *gJoystick = NULL;

// Total number of collectibles
int gCollectibleCount;
// Number of collectibles taken
int gCollectiblesTaken;
// Array of collectibles
collectible *gCollectible;

// Last iteration's tick value
int gLastTick;
// Level start tick
int gLevelStartTick;
// Current level
int gCurrentLevel;

// Player's position
float gXPos;
float gYPos;

// Player's motion vector
float gXMov;
float gYMov;

// Player's start position
float gStartX;
float gStartY;

// Player's key status
int gKeyLeft;
int gKeyRight;
int gKeyUp;
int gKeyDown;

// Player's score
int gScore;

// Level data
unsigned char gLevel[LEVELWIDTH * LEVELHEIGHT];  

// Offset of character data per character
int gFontOfs[256];
// Width of character data per character
int gFontWidth[256];


void init()
{
  if (SDL_NumJoysticks() > 0)
  {
    gJoystick = SDL_JoystickOpen(0);    
    if (SDL_JoystickNumAxes(gJoystick) < 2)
    {
      // Not enough axes for our use; don't use the joystick.
      SDL_JoystickClose(gJoystick);
      gJoystick = NULL;
    }
  }

  gCollectible = NULL;
  gScore = 0;
  gCurrentLevel = 0;

  SDL_Surface *temp = SDL_LoadBMP("tiles.bmp");
  gTiles = SDL_ConvertSurface(temp, gScreen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);

  temp = SDL_LoadBMP("font.bmp");
  gFont = SDL_ConvertSurface(temp, gScreen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);

  if (SDL_MUSTLOCK(gFont))
    if (SDL_LockSurface(gFont) < 0) 
      return;

  int i, j, k;
  for (k = 0; k < gFont->h / gFont->w; k++)
  {
    gFontOfs[k] = gFont->w;
    gFontWidth[k] = 0;
    for (i = 0; i < gFont->w; i++)
    {
      for (j = 0; j < gFont->w; j++)
      {
        if (((unsigned int*)gFont->pixels)[(i + k * gFont->w) * 
                                           (gFont->pitch / 4) + j] != 0) 
        {
          if (j < gFontOfs[k])
            gFontOfs[k] = j;

          if (j > gFontWidth[k])
            gFontWidth[k] = j;
        }
      }
    }
    gFontWidth[k] -= gFontOfs[k];
  }

  // Unlock if needed
  if (SDL_MUSTLOCK(gFont)) 
    SDL_UnlockSurface(gFont);

  reset();
}

void deinit()
{
  if (gJoystick)
    SDL_JoystickClose(gJoystick);
}

void render()
{
    rendergame();
}

// Entry point
int main(int argc, char *argv[])
{
  // Initialize SDL's subsystems
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) 
  {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }

  // Register SDL_Quit to be called at exit; makes sure things are
  // cleaned up when we quit.
  atexit(SDL_Quit);

  // make sure our deinit gets called at shutdown.
  atexit(deinit);
    
  // Attempt to create a WIDTHxHEIGHT window with 32bit pixels.
  gScreen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE);
  
  init();

  // If we fail, return error.
  if (gScreen == NULL) 
  {
    fprintf(stderr, "Unable to set up video: %s\n", SDL_GetError());
    exit(1);
  }

  // Main loop: loop forever.
  while (1)
  {
    // Render stuff
    render();

    // Poll for events, and handle the ones we care about.
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
      switch (event.type) 
      {
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
          gKeyLeft = 1;
          break;
        case SDLK_RIGHT:
          gKeyRight = 1;
          break;
        case SDLK_UP:
          gKeyUp = 1;
          break;
        case SDLK_DOWN:
          gKeyDown = 1;
          break;
        }
        break;
      case SDL_KEYUP:                  
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
          // If escape is pressed, return (and thus, quit)
          return 0;
        case SDLK_LEFT:
          gKeyLeft = 0;
          break;
        case SDLK_RIGHT:
          gKeyRight = 0;
          break;
        case SDLK_UP:
          gKeyUp = 0;
          break;
        case SDLK_DOWN:
          gKeyDown = 0;
          break;
        }
        break;
      case SDL_QUIT:
          return(0);
      }
    }
  }
  return 0;
}
