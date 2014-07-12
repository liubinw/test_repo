#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _MSC_VER
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif
#include "gp.h"

void rotate_z(float angle)
{
  float ca = (float)cos(angle);
  float sa = (float)sin(angle);
  int i;
  for (i = 0; i < BALLVTXCOUNT; i++)
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
  for (i = 0; i < BALLVTXCOUNT; i++)
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
  for (i = 0; i < BALLVTXCOUNT; i++)
  {
    float y = gRVtx[i].y * ca - gRVtx[i].z * sa;
    float z = gRVtx[i].y * sa + gRVtx[i].z * ca;
    gRVtx[i].y = y;
    gRVtx[i].z = z;
  }
}

void drawball(int x, int y, int r, int colorb, int color0, 
              int color1, float roty, float rotz)
{   
  drawcircle(x, y, r, colorb);

  rotate_z(-rotz);
  rotate_y(roty);
  rotate_z(rotz);

  int i;

  for (i = 0; i < BALLVTXCOUNT; i++)
  {
    if (gRVtx[i].z < 0)
      drawcircle((int)(gRVtx[i].x * (r - 2) + x),
                 (int)(gRVtx[i].y * (r - 2) + y),
                 2, color0);
  }

  for (i = 0; i < BALLVTXCOUNT; i++)
  {
    if (gRVtx[i].z >= 0)
      drawcircle((int)(gRVtx[i].x * (r - 2) + x),
                 (int)(gRVtx[i].y * (r - 2) + y),
                 2, color1);
  }
}