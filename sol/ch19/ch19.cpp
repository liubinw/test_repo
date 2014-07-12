#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif

// Screen surface
SDL_Surface *gScreen;
// Picture surface
SDL_Surface *gPicture;


// Screen pitch
#define PITCH (gScreen->pitch / 4)
// Screen width
#define WIDTH 480
// Screen height
#define HEIGHT 320
// Definition of PI
#define PI 3.1415926535897932384626433832795f


// Physics iterations per second
#define PHYSICSFPS 100

// Last iteration's tick value
int gLastTick;


void init()
{
  SDL_Surface *temp = SDL_LoadBMP("picture19.bmp");
  gPicture = SDL_ConvertSurface(temp, gScreen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);
}


void dist(float v)
{
  if (SDL_MUSTLOCK(gPicture))
    if (SDL_LockSurface(gPicture) < 0) 
      return;

  int i, j;
  
  int ypos = (int)(-pow(v, 5) * (HEIGHT + 64));
  
  int xdist[WIDTH];
  int ydist[HEIGHT];

  for (i = 0; i < WIDTH; i++)
  {
    xdist[i] = (int)((sin((v * (i + 10)) * 0.0024857) -
                      sin((v * (i + 23)) * 0.0324857) * 
                      sin((v * (i + 54)) * 0.0724857)) * pow(v, 3) * 32);
          
  }

  for (i = 0; i < HEIGHT; i++)
  {
    ydist[i] = (int)((sin((v * (i + 90)) * 0.0024857) -
                      sin((v * (i + 31)) * 0.0214857) * 
                      sin((v * (i + 57)) * 0.0514857)) * pow(v, 3) * 32);
  }

  for (i = 0; i < HEIGHT; i++)
  {
    for (j = 0; j < WIDTH; j++)
    {
      int u = j + ydist[i] + xdist[j];
      int v = i + xdist[j] + ydist[i] + ypos;

      if (u >= 0 && u < WIDTH &&
          v >= 0 && v < HEIGHT)
      {      
        ((unsigned int*)gScreen->pixels)[(j) + (i) * PITCH] = 
          ((unsigned int*)gPicture->pixels)[(u) + (v) * (gPicture->pitch / 4)];
      }
      else
      {
        ((unsigned int*)gScreen->pixels)[(j) + (i) * PITCH] = 0;
      }
    }
  }

    if (SDL_MUSTLOCK(gPicture)) 
        SDL_UnlockSurface(gPicture);
}

void render()
{   
  // Ask SDL for the time in milliseconds
  int tick = SDL_GetTicks();

  if (tick <= gLastTick) 
  {
    SDL_Delay(1);
    return;
  }

  while (gLastTick < tick)
  {
    // 'physics' here

    gLastTick += 1000 / PHYSICSFPS;
  }

  // Lock surface if needed
  if (SDL_MUSTLOCK(gScreen))
    if (SDL_LockSurface(gScreen) < 0) 
      return;

  // rendering here
  dist(((tick % 8000) / 4000.0f) - 1.0f);

  // Unlock if needed
  if (SDL_MUSTLOCK(gScreen)) 
    SDL_UnlockSurface(gScreen);

  // Tell SDL to update the whole gScreen
  SDL_UpdateRect(gScreen, 0, 0, WIDTH, HEIGHT);    
}


// Entry point
int main(int argc, char *argv[])
{
  // Initialize SDL's subsystems
  if (SDL_Init(SDL_INIT_VIDEO) < 0) 
  {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }

  // Register SDL_Quit to be called at exit; makes sure things are
  // cleaned up when we quit.
  atexit(SDL_Quit);
   
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
      case SDL_KEYUP:
                    
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
          // If escape is pressed, return (and thus, quit)
          return 0;
        }
        break;
      case SDL_QUIT:
        return(0);
      }
    }
  }
  return 0;
}
