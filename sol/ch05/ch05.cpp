#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif

SDL_Surface *screen;

unsigned int *tempbuf;

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

#define PITCH (screen->pitch / 4)

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

void drawsprite_add(int x, int y, unsigned int color)
{
    int i, j, c, yofs;
    yofs = y * (screen->pitch / 4) + x;
    for (i = 0, c = 0; i < 16; i++)
    {
        for (j = 0; j < 16; j++, c++)
        {
            if (sprite[c])
            {
                ((unsigned int*)screen->pixels)[yofs + j] = 
                    blend_add(((unsigned int*)screen->pixels)[yofs + j], color);
            }
        }
        yofs += (screen->pitch / 4);
    }
}

void drawsprite_mul(int x, int y, unsigned int color)
{
    int i, j, c, yofs;
    yofs = y * (screen->pitch / 4) + x;
    for (i = 0, c = 0; i < 16; i++)
    {
        for (j = 0; j < 16; j++, c++)
        {
            if (sprite[c])
            {
                ((unsigned int*)screen->pixels)[yofs + j] = 
                    blend_mul(((unsigned int*)screen->pixels)[yofs + j], color);
            }
        }
        yofs += (screen->pitch / 4);
    }
}

void init()
{
    tempbuf = new unsigned int[640*480];
}

void scaleblit()
{
    int i, j, yofs;
    yofs = 0;
    for (i = 0; i < 480; i++)
    {
        for (j = 0; j < 640; j++)
        {
            int c = (int)((i * 0.95f) + 12) * 640 +
                    (int)((j * 0.95f) + 16);
            ((unsigned int*)screen->pixels)[yofs + j] = 
                blend_avg(((unsigned int*)screen->pixels)[yofs + j], tempbuf[c]);
        }
        yofs += PITCH;
    }

}

void render()
{   
    // Lock surface if needed
    if (SDL_MUSTLOCK(screen))
        if (SDL_LockSurface(screen) < 0) 
            return;

    // Ask SDL for the time in milliseconds
    int tick = SDL_GetTicks();

    int i, d;
    for (i = 0; i < 128; i++)
    {
        d = tick + i * 4;
        drawsprite((int)(320 + sin(d * 0.0034f) * sin(d * 0.0134f) * 300),
                   (int)(240 + sin(d * 0.0033f) * sin(d * 0.0234f) * 220),
                   ((int)(sin((tick * 0.2 + i) * 0.234897f) * 127 + 128) << 16) |
                   ((int)(sin((tick * 0.2 + i) * 0.123489f) * 127 + 128) <<  8) |
                   ((int)(sin((tick * 0.2 + i) * 0.312348f) * 127 + 128) <<  0));
    }

    for (i = 0; i < 480; i++)
        memcpy(tempbuf + i * 640, 
               ((unsigned long*)screen->pixels) + 
               i * PITCH, 640 * 4);

    scaleblit();

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

