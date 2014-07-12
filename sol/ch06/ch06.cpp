#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif

SDL_Surface *screen;

#define PITCH (screen->pitch / 4)

#define TREECOUNT 64

float xy[TREECOUNT * 2];

void init()
{
    srand(0x7aa7);
    int i;
    for (i = 0; i < TREECOUNT; i++)
    {   
        int x = rand();
        int y = rand();
        xy[i * 2 + 0] = ((x % 10000) - 5000) / 1000.0f;
        xy[i * 2 + 1] = ((y % 10000) - 5000) / 1000.0f;
    }
}

#define PI 3.1415926535897932384626433832795f

void drawcircle(int x, int y, int r, int c)
{
    int i, j;
    for (i = 0; i < 2 * r; i++)
    {
        // vertical clipping: (top and bottom)
        if ((y - r + i) >= 0 && (y - r + i) < 480)
        {
            int len = (int)(sqrt(cos(0.5f * PI * (i - r) / r)) * r * 2);
            int xofs = x - len / 2;

            // left border
            if (xofs < 0)
            {
                len += xofs;
                xofs = 0;
            }

            // right border
            if (xofs + len >= 640)
            {
                len -= (xofs + len) - 640;
            }
            int ofs = (y - r + i) * PITCH + xofs;
            
            // note that len may be 0 at this point, 
            // and no pixels get drawn!
            for (j = 0; j < len; j++)
                ((unsigned int*)screen->pixels)[ofs + j] = c;
        }
    }
}


void render()
{   
    // Lock surface if needed
    if (SDL_MUSTLOCK(screen))
        if (SDL_LockSurface(screen) < 0) 
            return;

    // Clear the screen with a green color
    int i, j;
    for (i = 0; i < 480; i++)
    {
        for (j = 0; j < 640; j++)
        {
          *(((unsigned long*)screen->pixels) + i * PITCH + j) = 0x005f00;
        }
    }
               

    // Ask SDL for the time in milliseconds
    int tick = SDL_GetTicks();
    
    float pos_x = (float)sin(tick * 0.00037234f) * 2;
    float pos_y = (float)cos(tick * 0.00057234f) * 2;

    float shadow_x = (float)sin(tick * 0.0002934872f) * 16;
    float shadow_y = (float)cos(tick * 0.0001813431f) * 16;

    for (j = 0; j < TREECOUNT; j++)
    {   
        float x = xy[j * 2 + 0] + pos_x;
        float y = xy[j * 2 + 1] + pos_y;
        for (i = 0; i < 8; i++)
        {        
            drawcircle((int)(x * 200 + 320 + (i + 1) * shadow_x),
                       (int)(y * 200 + 240 + (i + 1) * shadow_y),
                       (10 - i) * 5,
                       0x1f4f1f);
        }
    }

    for (i = 0; i < 8; i++)
    {        
        for (j = 0; j < TREECOUNT; j++)
        {
            float x = xy[j * 2 + 0] + pos_x;
            float y = xy[j * 2 + 1] + pos_y;
            drawcircle((int)(x * (200 + i * 4) + 320),
                       (int)(y * (200 + i * 4) + 240),
                       (9 - i) * 5,
                       i * 0x030906 + 0x1f671f);
        }
    }
                
    // Unlock if needed
    if (SDL_MUSTLOCK(screen)) 
        SDL_UnlockSurface(screen);

    // Tell SDL to update the whole screen
    SDL_UpdateRect(screen, 0, 0, 640, 480);    
}


// Entry point
int main(int argc, char *argv[])
{
    // Initialize SDL's subsystems - in this case, only video.
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    // Register SDL_Quit to be called at exit; makes sure things are
    // cleaned up when we quit.
    atexit(SDL_Quit);
    
    // Attempt to create a 640x480 window with 32bit pixels.
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    
    // If we fail, return error.
    if ( screen == NULL ) 
    {
        fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
        exit(1);
    }

    init();

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