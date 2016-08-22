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
// Definition of PI
#define PI 3.1415926535897932384626433832795f


// Physics iterations per second
#define PHYSICSFPS 100

// Vertex structure
struct vertex
{
  float x, y, z;
};

// Original vertices
vertex *gVtx;
// Transformed vertices
vertex *gRVtx;


// Last iteration's tick value
int gLastTick;


void init()
{
  gVtx = new vertex[100];
  gRVtx = new vertex[100];

  int i;
  for (i = 0; i < 100; i++)
  {
    gVtx[i].x = (rand() % 32768) - 16384.0f;
    gVtx[i].y = (rand() % 32768) - 16384.0f;
    gVtx[i].z = (rand() % 32768) - 16384.0f;
    float len = (float)sqrt(gVtx[i].x * gVtx[i].x + 
                            gVtx[i].y * gVtx[i].y + 
                            gVtx[i].z * gVtx[i].z);
    if (len != 0)
    {
      gVtx[i].x /= len;
      gVtx[i].y /= len;
      gVtx[i].z /= len;
    }
  }
}

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

void rotate_z(float angle)
{
  float ca = (float)cos(angle);
  float sa = (float)sin(angle);
  int i;
  for (i = 0; i < 100; i++)
  {
    float x = gRVtx[i].x * ca - gRVtx[i].y * sa;
    float y = gRVtx[i].x * sa + gRVtx[i].y * ca;
    gRVtx[i].x = x;
    gRVtx[i].y = y;
  }
}

void rotate_y(float angle)
{
  float ca = (float)cos(angle);
  float sa = (float)sin(angle);
  int i;
  for (i = 0; i < 100; i++)
  {
    float z = gRVtx[i].z * ca - gRVtx[i].x * sa;
    float x = gRVtx[i].z * sa + gRVtx[i].x * ca;
    gRVtx[i].z = z;
    gRVtx[i].x = x;
  }
}

void rotate_x(float angle)
{
  float ca = (float)cos(angle);
  float sa = (float)sin(angle);
  int i;
  for (i = 0; i < 100; i++)
  {
    float y = gRVtx[i].y * ca - gRVtx[i].z * sa;
    float z = gRVtx[i].y * sa + gRVtx[i].z * ca;
    gRVtx[i].y = y;
    gRVtx[i].z = z;
  }
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
  drawrect(0, 0, WIDTH, HEIGHT, 0);

  drawcircle(WIDTH / 2, HEIGHT / 2, WIDTH / 4, 0x3f3f3f);

  memcpy(gRVtx, gVtx, sizeof(vertex) * 100);

  float rotz = tick * 0.0005f;
  float roty = tick * 0.002f;

  rotate_y(roty);
  rotate_z(rotz);

  int i;

  for (i = 0; i < 100; i++)
  {
    int c = 0xffffff;
    if (gRVtx[i].z < 0)
      c = 0x7f7f7f;
    drawcircle((int)(gRVtx[i].x * (WIDTH / 4) + WIDTH / 2),
               (int)(gRVtx[i].y * (WIDTH / 4) + HEIGHT / 2),
               2, c);
  } 

  drawcircle((int)((WIDTH / 3) * cos(rotz) + WIDTH / 2),
             (int)((WIDTH / 3) * sin(rotz) + HEIGHT / 2), 6, 0xffffff);


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
