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
// Heightmap surface
SDL_Surface *gHeightmap;
// Texture surface
SDL_Surface *gTexture;

// Lookup table
short *gLut;


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
  SDL_Surface *temp = SDL_LoadBMP("picture18.bmp");
  gPicture = SDL_ConvertSurface(temp, gScreen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);

  temp = SDL_LoadBMP("heightmap18.bmp");
  gHeightmap = SDL_ConvertSurface(temp, gScreen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);

  temp = SDL_LoadBMP("texture18.bmp");
  gTexture = SDL_ConvertSurface(temp, gScreen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);

  int i,j;
  gLut = new short[WIDTH * HEIGHT];
  memset(gLut, 0, sizeof(short) * WIDTH * HEIGHT);

  if (SDL_MUSTLOCK(gHeightmap))
    if (SDL_LockSurface(gHeightmap) < 0) 
      return;

  for (i = 1; i < HEIGHT - 1; i++)
  {
    for (j = 1; j < WIDTH - 1; j++)
    {
      int ydiff = 
        ((((unsigned int*)gHeightmap->pixels)[j + (i - 1) * (gHeightmap->pitch / 4)] & 0xff) -
         (((unsigned int*)gHeightmap->pixels)[j + (i + 1) * (gHeightmap->pitch / 4)] & 0xff));
      int xdiff = 
        ((((unsigned int*)gHeightmap->pixels)[j - 1 + i * (gHeightmap->pitch / 4)] & 0xff) -
         (((unsigned int*)gHeightmap->pixels)[j + 1 + i * (gHeightmap->pitch / 4)] & 0xff));

      gLut[i * WIDTH + j] = ((ydiff & 0xff) << 8) | (xdiff & 0xff);
    }
  }

    if (SDL_MUSTLOCK(gHeightmap)) 
        SDL_UnlockSurface(gHeightmap);
}

unsigned int blend_add(unsigned int source, unsigned int target)
{
  unsigned int sourcer = (source >>  0) & 0xff;
  unsigned int sourceg = (source >>  8) & 0xff;
  unsigned int sourceb = (source >> 16) & 0xff;
  unsigned int targetr = (target >>  0) & 0xff;
  unsigned int targetg = (target >>  8) & 0xff;
  unsigned int targetb = (target >> 16) & 0xff;

  targetr += sourcer;
  targetg += sourceg;
  targetb += sourceb;

  if (targetr > 0xff) targetr = 0xff;
  if (targetg > 0xff) targetg = 0xff;
  if (targetb > 0xff) targetb = 0xff;

  return (targetr <<  0) |
         (targetg <<  8) |
         (targetb << 16);
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
  
  if (SDL_MUSTLOCK(gPicture))
    if (SDL_LockSurface(gPicture) < 0) 
      return;
  
  if (SDL_MUSTLOCK(gTexture))
    if (SDL_LockSurface(gTexture) < 0) 
      return;

  int posx = (int)((sin(tick * 0.000645234) + 1) * WIDTH / 4);
  int posy = (int)((sin(tick * 0.000445234) + 1) * HEIGHT / 4);
  int i, j;
  for (i = 0; i < HEIGHT; i++)
  {
    for (j = 0; j < WIDTH; j++)
    {
      int u = j + ((signed char)gLut[i * WIDTH + j]) - posx;
      int v = i + (gLut[i * WIDTH + j] / 256) - posy;
      if (v < 0 || v >= gTexture->w ||
          u < 0 || u >= gTexture->h)
      {
        ((unsigned int*)gScreen->pixels)[(j) + (i) * PITCH] = 
          ((unsigned int*)gPicture->pixels)[(j) + (i) * (gPicture->pitch / 4)];
      }
      else
      {
        ((unsigned int*)gScreen->pixels)[(j) + (i) * PITCH] = 
          blend_add(
          ((unsigned int*)gPicture->pixels)[(j) + (i) * (gPicture->pitch / 4)],
          ((unsigned int*)gTexture->pixels)[(u) + (v) * (gTexture->pitch / 4)]);
      }
    }
  }

    if (SDL_MUSTLOCK(gTexture)) 
        SDL_UnlockSurface(gTexture);

    if (SDL_MUSTLOCK(gPicture)) 
        SDL_UnlockSurface(gPicture);

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
