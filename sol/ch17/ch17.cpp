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
// Texture surface
SDL_Surface *gTexture;

// Look-up table
unsigned short *gLut;
// Distance mask
unsigned int *gMask; 


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
  SDL_Surface *temp = SDL_LoadBMP("texture17.bmp");
  gTexture = SDL_ConvertSurface(temp, gScreen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);

  gLut = new unsigned short[WIDTH * HEIGHT * 4];
  gMask = new unsigned int[WIDTH * HEIGHT * 4];

  int i,j;
  for (i = 0; i < HEIGHT * 2; i++)
  {
    for (j = 0; j < WIDTH * 2; j++)
    {
      int xdist = j - (WIDTH);
      int ydist = i - (HEIGHT);
     
      // round
      int distance = (int)sqrt((float)(xdist * xdist + ydist * ydist));
  
      // square
      //int distance = (abs(xdist) > abs(ydist)) ? abs(xdist) : abs(ydist);
  
      // diamond
      //int distance = (abs(xdist) + abs(ydist)) / 2;
     
      // flower
      //distance += (int)(sin(atan2((float)xdist, (float)ydist) * 5) * 8);
     
      if (distance <= 0) distance = 1;
  
      int d = distance;
      if (d > 255) d = 255;
      gMask[i * WIDTH * 2 + j] = d * 0x010101;
     
      distance = (64 * 256 / distance) & 0xff;
  
      int angle = (int)(((atan2((float)xdist, (float)ydist) / PI) + 1.0f) * 128);
  
      gLut[i * WIDTH * 2 + j] = (distance << 8) + angle;
    }
  }
}


unsigned int blend_mul(unsigned int source, unsigned int target)
{
  unsigned int sourcer = (source >>  0) & 0xff;
  unsigned int sourceg = (source >>  8) & 0xff;
  unsigned int sourceb = (source >> 16) & 0xff;
  unsigned int targetr = (target >>  0) & 0xff;
  unsigned int targetg = (target >>  8) & 0xff;
  unsigned int targetb = (target >> 16) & 0xff;

  targetr = (sourcer * targetr) >> 8;
  targetg = (sourceg * targetg) >> 8;
  targetb = (sourceb * targetb) >> 8;

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

  if (SDL_MUSTLOCK(gTexture))
    if (SDL_LockSurface(gTexture) < 0) 
      return;

  int posx = (int)((sin(tick * 0.000645234f) + 1) * WIDTH / 2);
  int posy = (int)((sin(tick * 0.000445234f) + 1) * HEIGHT / 2);
  int posx2 = (int)((sin(-tick * 0.000645234f) + 1) * WIDTH / 2);
  int posy2 = (int)((sin(-tick * 0.000445234f) + 1) * HEIGHT / 2);

  int i, j;
  for (i = 0; i < HEIGHT; i++)
  {
    for (j = 0; j < WIDTH; j++)
    {
      int lut = gLut[(i + posy) * WIDTH * 2 + j + posx] - 
                gLut[(i + posy2) * WIDTH * 2 + j + posx2];
      int mask = gMask[(i + posy) * WIDTH * 2 + j + posx];
      int mask2 = gMask[(i + posy2) * WIDTH * 2 + j + posx2];

      ((unsigned int *)gScreen->pixels)[(j) + (i) * PITCH] = 
        blend_mul(
        blend_mul(
        ((unsigned int*)gTexture->pixels)[((lut + tick / 32) & 0xff) + 
                                         (((lut >> 8) + tick / 8) & 0xff) * 
                                         (gTexture->pitch / 4)],
         mask),
         mask2);
    }
  }

  if (SDL_MUSTLOCK(gTexture)) 
    SDL_UnlockSurface(gTexture);

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
