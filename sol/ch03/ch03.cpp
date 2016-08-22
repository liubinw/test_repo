#include <stdlib.h>
#include <math.h>
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif

SDL_Surface *screen;

void putpixel(int x, int y, int color)
{
    unsigned int *ptr = (unsigned int*)screen->pixels;
    int lineoffset = y * (screen->pitch / 4);
    ptr[lineoffset + x] = color;
}

const unsigned char sprite[] = 
{
0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0
};

void drawsprite(int x, int y, unsigned int color)
{
    int i, j, c, yofs;
    yofs = y * (screen->pitch / 4) + x;
    for (i = 0, c = 0; i < 16; i++)
    {
        for (j = 0; j < 16; j++, c++)
        {
            if (sprite[c])
            {
                ((unsigned int*)screen->pixels)[yofs + j] = color;
            }
        }
        yofs += (screen->pitch / 4);
    }
}

void render()
{   
    // Lock surface if needed
    if (SDL_LockSurface(screen) < 0) 
        return;

    // Ask SDL for the time in milliseconds
    int tick = SDL_GetTicks();

    // Declare a couple of variables
    int i, j, d, yofs, ofs;

    // Draw to screen
    for (i = 0, yofs = 0, d = 0; i < 480; i++, yofs += screen->pitch / 4)
        for (j = 0, ofs = yofs; j < 640; j++, ofs++, d++)
            ((unsigned int*)screen->pixels)[ofs] = 0;

    for (i = 0; i < 128; i++)
    {
        d = tick + i * 10;
        drawsprite((int)(320 + sin(d * 0.003459734f) * 300),
                   (int)(240 + sin(d * 0.003345973f) * 220),
                   ((int)(sin((tick * 0.2 + i) * 0.234897f) * 127 + 128) << 16) |
                   ((int)(sin((tick * 0.2 + i) * 0.123489f) * 127 + 128) <<  8) |
                   ((int)(sin((tick * 0.2 + i) * 0.312348f) * 127 + 128) <<  0));
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

