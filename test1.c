
#include "SDL/SDL.h" 

int main( int argc, char* args[] ) 
{ 
    SDL_Surface* hello = NULL;
    SDL_Surface* screen = NULL;


        //Start SDL 
	SDL_Init( SDL_INIT_EVERYTHING ); 

	printf("Testing\n");

SDL_Init( SDL_INIT_EVERYTHING );
screen = SDL_SetVideoMode( 640, 480, 32, SDL_SWSURFACE );
hello = SDL_LoadBMP( "hello.bmp" );


SDL_BlitSurface( hello, NULL, screen, NULL );
SDL_Flip( screen );
SDL_Delay( 2000 );
SDL_FreeSurface( hello );



	//Quit SDL 
	SDL_Quit(); 
	return 0; 
} 
