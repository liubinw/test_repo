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
// Surface that contains the tiles
SDL_Surface *gTiles;
// Surface that contains the font
SDL_Surface *gFont;


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
// Definition of PI
#define PI 3.1415926535897932384626433832795f

// Width of level data
#define LEVELWIDTH 15
// Height of level data
#define LEVELHEIGHT 10
// Fall color
#define FALLCOLOR 0x000000
// Tile size (width and height)
#define TILESIZE 32

// Player's thrust value
#define THRUST 0.1f
// Slowdown due to friction, etc.
#define SLOWDOWN 0.99f
// Slowdown due to collision
#define COLLISIONSLOWDOWN 0.9f

// Color of the start tile
#define STARTCOLOR 0x001f7f
// Color of the goal tile
#define ENDCOLOR 0x3f7f1f
// Radius of a collectible item
#define COLLECTIBLERADIUS 8
// Color of a collectible item
#define COLLECTIBLECOLOR 0xffff00


// Physics iterations per second
#define PHYSICSFPS 100

struct collectible
{
  float mX;
  float mY;
  int mColor;
  int mRadius;
  int mTaken;
};

// Total number of collectibles
int gCollectibleCount;
// Number of collectibles taken
int gCollectiblesTaken;
// Array of collectibles
collectible *gCollectible;

// Last iteration's tick value
int gLastTick;

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

// Tile values
enum leveldataenum
{
  LEVEL_DROP = 0,
  LEVEL_GROUND = 1,
  LEVEL_START = 2,
  LEVEL_END = 3,
  LEVEL_COLLECTIBLE = 5
};

// Level data
unsigned char gLevel[LEVELWIDTH * LEVELHEIGHT] =
{
  0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
  0,0,0,1,1,1,5,1,1,5,1,1,1,3,0,
  0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,
  0,1,1,1,5,1,1,1,1,1,1,1,1,1,1,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,5,
  0,1,0,0,0,0,1,1,1,1,1,5,1,1,1,
  1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,0,1,1,5,1,1,0,1,0,1,
  1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,
  1,1,2,1,1,0,1,0,1,1,1,0,1,1,1
};  

// Offset of character data per character
int gFontOfs[256];
// Width of character data per character
int gFontWidth[256];


unsigned int blend_avg(unsigned int source, unsigned int target)
{
    unsigned int sourcer = (source >>  0) & 0xff;
    unsigned int sourceg = (source >>  8) & 0xff;
    unsigned int sourceb = (source >> 16) & 0xff;
    unsigned int targetr = (target >>  0) & 0xff;
    unsigned int targetg = (target >>  8) & 0xff;
    unsigned int targetb = (target >> 16) & 0xff;

    targetr = (sourcer + targetr) / 2;
    targetg = (sourceg + targetg) / 2;
    targetb = (sourceb + targetb) / 2;

    return (targetr <<  0) |
           (targetg <<  8) |
           (targetb << 16);
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

void drawtile(int x, int y, int tile)
{
  // Lock surface if needed
  if (SDL_MUSTLOCK(gTiles))
    if (SDL_LockSurface(gTiles) < 0) 
      return;

  int i, j;
  for (i = 0; i < TILESIZE; i++)
  {
    int screenofs = x + (y + i) * PITCH;
    int tileofs = (i + tile * TILESIZE) * (gTiles->pitch / 4);
    for (j = 0; j < TILESIZE; j++)
    {
      ((unsigned int*)gScreen->pixels)[screenofs] = 
        ((unsigned int*)gTiles->pixels)[tileofs];
      screenofs++;
      tileofs++;
    }
  }

  // Unlock if needed
  if (SDL_MUSTLOCK(gTiles)) 
    SDL_UnlockSurface(gTiles);
}

void drawcharacter(int x, int y, int character, int blend)
{
  if (character == 32)
    return;
  
  character -= 33; // our font does not have the first 33 characters.

  // Lock surface if needed
  if (SDL_MUSTLOCK(gFont))
    if (SDL_LockSurface(gFont) < 0) 
      return;

  int i, j;
  for (i = 0; i < gFont->w; i++)
  {
    int screenofs = x + (y + i) * PITCH;
    int charofs = (i + character * gFont->w) * 
                (gFont->pitch / 4) + gFontOfs[character];
    for (j = 0; j < gFontWidth[character]; j++)
    {
      int pixel = ((unsigned int*)gFont->pixels)[charofs];
      if (pixel != 0)
      {
        int oldpixel = ((unsigned int*)gScreen->pixels)[screenofs];
        switch (blend)
        {
        case -1:
          pixel = blend_mul(pixel ^ 0xffffff, oldpixel);
          break;
        case 0:
          pixel = blend_avg(pixel, oldpixel);
          break;
        case 1:
          pixel = blend_add(pixel, oldpixel);
          break;
        }
        ((unsigned int*)gScreen->pixels)[screenofs] = pixel;        
      }
      screenofs++;
      charofs++;
    }
  }

  // Unlock if needed
  if (SDL_MUSTLOCK(gFont)) 
    SDL_UnlockSurface(gFont);
}

void drawstring(int x, int y, char *s)
{
  while (*s != 0)
  {
    drawcharacter(x + 1, y + 1, *s, -1);
    drawcharacter(x, y, *s, 0);
    if (*s == 32)
      x += gFont->w / 2;
    else
      x += gFontWidth[*s - 33];
    s++;
  }
}


void collectiblecollision()
{
  int i;
  for (i = 0; i < gCollectibleCount; i++)
  {
    if (gCollectible[i].mTaken == 0)
    {
      if (sqrt((gCollectible[i].mX - gXPos) * 
               (gCollectible[i].mX - gXPos) +
             (gCollectible[i].mY - gYPos) * 
             (gCollectible[i].mY - gYPos)) <
          RADIUS + gCollectible[i].mRadius)
      {
        gCollectiblesTaken++;
        gCollectible[i].mTaken = 1;
      }
    }
  }
}

void reset()
{
  gXMov = 0;
  gYMov = 0;

  gKeyLeft = 0;
  gKeyRight = 0;
  gKeyUp = 0;
  gKeyDown = 0;

  gCollectibleCount = 0;
  int i;
  for (i = 0; i < LEVELWIDTH * LEVELHEIGHT; i++)
  {
    if (gLevel[i] == LEVEL_START)
    {
      gStartX = (float)((i % LEVELWIDTH) * TILESIZE + TILESIZE / 2);
      gStartY = (float)((i / LEVELWIDTH) * TILESIZE + TILESIZE / 2);
    }
    if (gLevel[i] == LEVEL_COLLECTIBLE)
      gCollectibleCount++;
  }
  
  delete[] gCollectible;
  gCollectible = new collectible[gCollectibleCount];
  gCollectibleCount = 0;
  for (i = 0; i < LEVELWIDTH * LEVELHEIGHT; i++)
  {
    if (gLevel[i] == LEVEL_COLLECTIBLE)
    {
      gCollectible[gCollectibleCount].mX = 
        (float)((i % LEVELWIDTH) * TILESIZE + TILESIZE / 2);
      gCollectible[gCollectibleCount].mY = 
        (float)((i / LEVELWIDTH) * TILESIZE + TILESIZE / 2);
      gCollectible[gCollectibleCount].mColor = COLLECTIBLECOLOR;
      gCollectible[gCollectibleCount].mRadius = COLLECTIBLERADIUS;
      gCollectible[gCollectibleCount].mTaken = 0;
      gCollectibleCount++;
    }
  }

  gCollectiblesTaken = 0;

  gXPos = gStartX;
  gYPos = gStartY;

  gLastTick = SDL_GetTicks(); 
}

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

    switch (gLevel[(((int)gYPos) / TILESIZE) * LEVELWIDTH + ((int)gXPos) / TILESIZE])
    {
    case LEVEL_DROP:
      // player fell off - reset position
      reset();
      break;
    case LEVEL_END:
      if (gCollectibleCount == gCollectiblesTaken)
      {
        // All collectibles taken and end reached. Player wins.
        reset();
      }
      break;
    }

    if (gLevel[(((int)gYPos) / TILESIZE) * LEVELWIDTH + ((int)gXPos) / TILESIZE] == 0)
    {
      // player fell off - reset position
      gXPos = WIDTH / 2;
      gYPos = HEIGHT / 2;
    }

    gXMov *= SLOWDOWN;
    gYMov *= SLOWDOWN;

    gXPos += gXMov;
    gYPos += gYMov;

    // Collision with the screen borders
    if (gXPos > WIDTH - RADIUS)
    {
      gXPos -= gXMov;
      gXMov = -gXMov * COLLISIONSLOWDOWN;
    }

    if (gXPos < RADIUS)
    {
      gXPos -= gXMov;
      gXMov = -gXMov * COLLISIONSLOWDOWN;
    }

    if (gYPos > HEIGHT - RADIUS)
    {
      gYPos -= gYMov;
      gYMov = -gYMov * COLLISIONSLOWDOWN;
    }

    if (gYPos < RADIUS)
    {
      gYPos -= gYMov;
      gYMov = -gYMov * COLLISIONSLOWDOWN;
    }

    collectiblecollision();

    gLastTick += 1000 / PHYSICSFPS;
  }

  // Lock surface if needed
  if (SDL_MUSTLOCK(gScreen))
    if (SDL_LockSurface(gScreen) < 0) 
      return;

  // fill background
  int i, j;
  for (i = 0; i < LEVELHEIGHT; i++)
  {
    for (j = 0; j < LEVELWIDTH; j++)
    {
      switch (gLevel[i * LEVELWIDTH + j])
      {
      case LEVEL_DROP:
        drawrect(j * TILESIZE, i * TILESIZE, TILESIZE, TILESIZE, FALLCOLOR);
        break;
      case LEVEL_GROUND:
      case LEVEL_COLLECTIBLE:
        drawtile(j * TILESIZE, i * TILESIZE, 0);
        break;
      case LEVEL_START:
        drawtile(j * TILESIZE, i * TILESIZE, 2);
        break;
      case LEVEL_END:
        drawtile(j * TILESIZE, i * TILESIZE, 1);
        break;
      }
    }
  }

  // draw the collectibles
  for (i = 0; i < gCollectibleCount; i++)
  {
    if (gCollectible[i].mTaken == 0)
    {
      drawcircle((int)gCollectible[i].mX+2,
             (int)gCollectible[i].mY+2,
             gCollectible[i].mRadius,
             0);
      drawcircle((int)gCollectible[i].mX,
             (int)gCollectible[i].mY,
             gCollectible[i].mRadius,
             gCollectible[i].mColor);
    }
  }

  // draw the player object
  drawcircle((int)gXPos + 2,
             (int)gYPos + 2,
             RADIUS,
             0);
  drawcircle((int)gXPos,
             (int)gYPos,
             RADIUS,
             BALLCOLOR);
        

  char scorestring[80];
  sprintf(scorestring, "Score:%d", gCollectiblesTaken);
  drawstring(5, 5, scorestring);

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