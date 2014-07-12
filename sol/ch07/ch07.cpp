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

// Screen pitch
#define PITCH (gScreen->pitch / 4)
// Screen width
#define WIDTH 480
// Screen height
#define HEIGHT 320
// Color of player's ball
#define BALLCOLOR 0x007fff
// Background color
#define BGCOLOR 0x5f0000
// Wall color
#define WALLCOLOR 0x9f1f1f
// Definition of PI
#define PI 3.1415926535897932384626433832795f

// Player's position
float gXPos;
float gYPos;


void drawcircle(int x, int y, int r, int c)
{
  int i, j;
  for (i = 0; i < 2 * r; i++)
  {
    // vertical clipping: (top and bottom)
    if ((y - r + i) >= 0 && (y - r + i) < HEIGHT)
    {
      int len = (int)sqrt((float)(r * r - (r - i) * (r - i))) * 2;
      int xofs = x - len / 2;

      // left border
      if (xofs < 0)
      {
        len += xofs;
        xofs = 0;
      }

      // right border
      if (xofs + len >= WIDTH)
      {
        len -= (xofs + len) - WIDTH;
      }
      int ofs = (y - r + i) * PITCH + xofs;
      
      // note that len may be 0 at this point, 
      // and no pixels get drawn!
      for (j = 0; j < len; j++)
        ((unsigned int*)gScreen->pixels)[ofs + j] = c;
    }
  }
}


void drawrect(int x, int y, int width, int height, int c)
{
  int i, j;
  for (i = 0; i < height; i++)
  {
    // vertical clipping: (top and bottom)
    if ((y + i) >= 0 && (y + i) < HEIGHT)
    {
      int len = width;
      int xofs = x;

      // left border
      if (xofs < 0)
      {
        len += xofs;
        xofs = 0;
      }

      // right border
      if (xofs + len >= WIDTH)
      {
        len -= (xofs + len) - WIDTH;
      }
      int ofs = (i + y) * PITCH + xofs;

      // note that len may be 0 at this point, 
      // and no pixels get drawn!
      for (j = 0; j < len; j++)
        ((unsigned int*)gScreen->pixels)[ofs + j] = c;
    }
  }
}


void init()
{
  gXPos = WIDTH / 2;
  gYPos = HEIGHT / 2;
}


void render()
{   
  // Ask SDL for the time in milliseconds
  int tick = SDL_GetTicks();
  
  // Lock surface if needed
  if (SDL_MUSTLOCK(gScreen))
    if (SDL_LockSurface(gScreen) < 0) 
      return;

  // fill background
  drawrect(0, 0, WIDTH, HEIGHT, BGCOLOR);  
  
  // draw borders
  drawrect(0, 0, WIDTH, 8, WALLCOLOR);         
  drawrect(0, 0, 8, HEIGHT, WALLCOLOR);         
  drawrect(WIDTH - 8, 0, 8, HEIGHT, WALLCOLOR);         
  drawrect(0, HEIGHT - 8, WIDTH, 8, WALLCOLOR);         

  // draw the player object
  drawcircle((int)gXPos,
             (int)gYPos,
             12,
             BALLCOLOR);
        
  // Unlock if needed
    if (SDL_MUSTLOCK(gScreen)) 
        SDL_UnlockSurface(gScreen);

  // Tell SDL to update the whole gScreen
    SDL_UpdateRect(gScreen, 0, 0, WIDTH, HEIGHT);    
}


// Entry point
int main(int argc, char *argv[])
{
  // Initialize SDL's subsystems - in this case, only video.
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
      case SDL_KEYDOWN:
        break;
      case SDL_KEYUP:
        // If escape is pressed, return (and thus, quit)
        if (event.key.keysym.sym == SDLK_ESCAPE)
            return 0;
        break;
      case SDL_QUIT:
        return(0);
      }
    }
  }
  return 0;
}

