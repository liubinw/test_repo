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

// Pointer to the joystick interface
SDL_Joystick *gJoystick = NULL;


// Screen pitch
#define PITCH (gScreen->pitch / 4)
// Screen width
#define WIDTH 480
// Screen height
#define HEIGHT 320
// Color of player's ball
#define BALLCOLOR 0x007fff
// Radius of the player's ball
#define RADIUS 12
// Background color
#define BGCOLOR 0x5f0000
// Wall color
#define WALLCOLOR 0x9f1f1f
// Definition of PI
#define PI 3.1415926535897932384626433832795f

// Player's thrust value
#define THRUST 0.1f
// Slowdown due to friction, etc.
#define SLOWDOWN 0.99f

// Physics iterations per second
#define PHYSICSFPS 100


// Last iteration's tick value
int gLastTick;

// Player's position
float gXPos;
float gYPos;

// Player's motion vector
float gXMov;
float gYMov;

// Player's key status
int gKeyLeft;
int gKeyRight;
int gKeyUp;
int gKeyDown;


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

  gXMov = 0;
  gYMov = 0;

  gKeyLeft = 0;
  gKeyRight = 0;
  gKeyUp = 0;
  gKeyDown = 0;

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

  gLastTick = SDL_GetTicks(); 
}

void deinit()
{
  if (gJoystick)
    SDL_JoystickClose(gJoystick);
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
    if (gKeyLeft) gXMov -= THRUST;
    if (gKeyRight) gXMov += THRUST;
    if (gKeyUp) gYMov -= THRUST;
    if (gKeyDown) gYMov += THRUST;

    if (gJoystick)
    {
      gXMov += (SDL_JoystickGetAxis(gJoystick, 0) / 32768.0f) * THRUST;
      gYMov += (SDL_JoystickGetAxis(gJoystick, 1) / 32768.0f) * THRUST;
    }

    gXMov *= SLOWDOWN;
    gYMov *= SLOWDOWN;

    gXPos += gXMov;
    gYPos += gYMov;

    gLastTick += 1000 / PHYSICSFPS;
  }

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
             RADIUS,
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
