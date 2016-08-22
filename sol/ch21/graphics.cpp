#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _MSC_VER
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif
#include "gp.h"

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