#if 0
#!/bin/bash
APP_NAME="sdltest"
gcc -o bin/$APP_NAME \
-I/Library/Frameworks/SDL.framework/Headers \
-I/System/Library/Frameworks/OpenGL.framework/Headers \
$APP_NAME.c \
SDLmain.m \
-framework SDL \
-framework Cocoa \
-framework OpenGL
if [ "$?" = "0" ]; then
	time ./bin/$APP_NAME
fi
exit
#endif

#include <assert.h>
#include <string.h>
#include <math.h>
#include "SDL/SDL.h"

#define WIDTH 640
#define HEIGHT 480
#define BYTES_PER_PIXEL 4
#define DEPTH 32
#define FULLSCREEN 0	// Set this to 1 to start in full screen.

// Sets a single pixel on the screen.
void screen_SetPixel
(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	assert(screen != NULL);
	if (x < 0 || y < 0 || x >= screen->w || y >= screen->h) return;
	
    Uint32 *pixmem32;
    Uint32 colour;
	int stride = screen->pitch / BYTES_PER_PIXEL;
	
    colour = SDL_MapRGB(screen->format, r, g, b);
	
    pixmem32 = (Uint32*)screen->pixels + x + y * stride;
    *pixmem32 = colour;
}

// Displays some colors on the screen as a demo.
void demo_ShowSomeColors(SDL_Surface *screen, int frame_counter) {
	const int f = frame_counter;
	int x, y;
    for (y = 0; y < screen->h; y++) {
        for(x = 0; x < screen->w; x++) {
            screen_SetPixel
			(screen, x, y, (x*x)/256+3*y+f, (y*y)/256+x+f, f);
        }
    }
}

void Load(void)
{
	// Do loading code here.
}

void Unload(void)
{
	// Release resources and do saving here.
}

// The method that draws graphics to the screen.
void DrawScreen(SDL_Surface* screen, const int frame_counter)
{
    if(SDL_MUSTLOCK(screen)) {
        if(SDL_LockSurface(screen) < 0) return;
    }
	
	// Add drawing code here.
	demo_ShowSomeColors(screen, frame_counter);
	
    if(SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}
	
    SDL_Flip(screen);
}

int main( int argc, char* args[] ) {
	// Start SDL.
	SDL_Init( SDL_INIT_EVERYTHING );
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;
	
	SDL_Surface *screen = NULL;
	screen = FULLSCREEN
	? SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_FULLSCREEN|SDL_HWSURFACE)
	: SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE);
    if (!screen) {
        SDL_Quit();
        return 1;
    }
	
	Load();
	
	int quit = 0;
	int frame_counter = 0;
	SDL_Event event;
	while(!quit) {
		DrawScreen(screen, frame_counter++);
		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_KEYDOWN:
					quit = 1;
					break;
			}
		}
    }
	
	Unload();
	
	// Quit SDL.
	SDL_Quit();
	return 0;
}


