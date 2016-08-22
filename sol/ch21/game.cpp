#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _MSC_VER
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif
#include "gp.h"

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

  char name[80];
  FILE * f;
  do
  {
    sprintf(name, "level%d.txt", gCurrentLevel);
    f = fopen(name, "rb");
    if (f == NULL)
    {
      if (gCurrentLevel == 0)
        exit(0);
      gCurrentLevel = 0;
    }
  }
  while (f == NULL);

  int p = 0;
  while (p < LEVELWIDTH * LEVELHEIGHT && !feof(f))
  {
    int v = fgetc(f);
    if (v > 32)
    {
      switch (v)
      {
      case '.':
        gLevel[p] = LEVEL_DROP;
        break;
      case '_':
        gLevel[p] = LEVEL_GROUND;
        break;
      case 'S':
        gLevel[p] = LEVEL_START;
        break;
      case 'E':
        gLevel[p] = LEVEL_END;
        break;
      case 'o':
        gLevel[p] = LEVEL_COLLECTIBLE;
        break;
      case '>':
        gLevel[p] = LEVEL_RIGHT;
        break;
      case '<':
        gLevel[p] = LEVEL_LEFT;
        break;
      case 'v':
        gLevel[p] = LEVEL_DOWN;
        break;
      case '^':
        gLevel[p] = LEVEL_UP;
        break;
      }
      p++;
    }
  }
  fclose(f);

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
  gLevelStartTick = SDL_GetTicks(); 
}

void rendergame()
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
      gScore += 100;
      if (gCollectibleCount > 0)
      {
        int secondsleft = TIMELIMIT - (gLastTick - gLevelStartTick) / 1000;
        if (secondsleft < 0)
          secondsleft = 0;
        gScore += (secondsleft * 20) * gCollectiblesTaken / gCollectibleCount;
      }
      gCurrentLevel++;
      reset();
      break;
    case LEVEL_LEFT:
      gXMov -= SLIDEPOWER;
      break;
    case LEVEL_RIGHT:
      gXMov += SLIDEPOWER;
      break;
    case LEVEL_UP:
      gYMov -= SLIDEPOWER;
      break;
    case LEVEL_DOWN:
      gYMov += SLIDEPOWER;
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
      case LEVEL_UP:
        drawtile(j * TILESIZE, i * TILESIZE, 4);
        break;
      case LEVEL_RIGHT:
        drawtile(j * TILESIZE, i * TILESIZE, 5);
        break;
      case LEVEL_DOWN:
        drawtile(j * TILESIZE, i * TILESIZE, 6);
        break;
      case LEVEL_LEFT:
        drawtile(j * TILESIZE, i * TILESIZE, 7);
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
  sprintf(scorestring, "Score:%d", gScore);
  drawstring(5, 5, scorestring);

  int secondsleft = TIMELIMIT - (gLastTick - gLevelStartTick) / 1000;
  if (secondsleft < 0)
    secondsleft = 0;
  sprintf(scorestring, "Time:%d", secondsleft);
  drawstring(5, 22, scorestring);

  // Unlock if needed
    if (SDL_MUSTLOCK(gScreen)) 
        SDL_UnlockSurface(gScreen);

  // Tell SDL to update the whole gScreen
    SDL_UpdateRect(gScreen, 0, 0, WIDTH, HEIGHT);    
}
