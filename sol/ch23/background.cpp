#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _MSC_VER
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif
#include "gp.h"

void drawbackground(int tick, int posx, int posy)
{     
  int i, j;
  for (i = 0; i < HEIGHT; i++)
  {
    for (j = 0; j < WIDTH; j++)
    {
      int lut = gLut[(i + posy) * WIDTH * 2 + j + posx];
      int mask = gMask[(i + posy) * WIDTH * 2 + j + posx];

      ((unsigned int *)gScreen->pixels)[(j) + (i) * PITCH] = 
        blend_mul(
        ((unsigned int*)gTexture->pixels)[((lut + tick / 32) & 0xff) + 
                                         (((lut >> 8) + tick / 16) & 0xff) * 
                                         (gTexture->pitch / 4)],
         mask);
    }
  }
}