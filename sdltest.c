#if 0
#!/bin/bash
APP_NAME="sdltest"
gcc -o bin/$APP_NAME \
-I/Library/Frameworks/SDL.framework/Headers \
-I/Library/Frameworks/SDL_image.framework/Headers \
-I/System/Library/Frameworks/OpenGL.framework/Headers \
$APP_NAME.c \
SDLmain.m \
-framework SDL \
-framework SDL_image \
-framework Cocoa \
-framework OpenGL
if [ "$?" = "0" ]; then
	time ./bin/$APP_NAME
fi
exit
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gl.h>
#include <glu.h>
#include <assert.h>
#include "SDL_image.h"
#include "SDL/SDL.h"

#define WIDTH 640
#define HEIGHT 480
#define BYTES_PER_PIXEL 4
#define DEPTH 32
#define FULLSCREEN 0	// Set this to 1 to start in full screen.

#define TEXTURES_CAP 1024	// Maximum number of textures.
#define SPRITES_CAP 4096	// Maximum number of sprites.
#define LINE_CAP 1024		// Maximum number of characters per line in file.

// Array of textures.
int textures_length;
GLuint *textures;
int *textures_width;
int *textures_height;

// Array of sprites.
// A sprite is a specific rectangle of any texture.
// The coordinates goes from 0 to 1.
int sprites_length;
int *sprites_texture_ids;
float *sprites_xs;
float *sprites_ys;
float *sprites_xs2;
float *sprites_ys2;

int face;
int test_sprite;

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

// Loads texture from file.
// The directory is relative to the running executable.
int LoadTexture(char *file)
{
    GLuint TextureID = 0;
	
	SDL_Surface* Surface = IMG_Load(file);
	
	assert(Surface != NULL);
	
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	
	// TODO: Swap red with blue color channels.
	int Mode = GL_RGB;
	if(Surface->format->BytesPerPixel == 4) {
		Mode = GL_RGBA;
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, Mode, Surface->w, Surface->h, 0, Mode, GL_UNSIGNED_BYTE, Surface->pixels);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// Quit application if running out of textures.
	if (textures_length >= TEXTURES_CAP) {
		printf("Maximum number of textures reached\n");
		exit(1);
	}
	
	int id = textures_length;
	textures[id] = TextureID;
	textures_width[id] = Surface->w;
	textures_height[id] = Surface->h;
	textures_length++;
	return id;
}

void DrawImageRect(int texture_id, float x, float y, float w, float h) {
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[texture_id]);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(x, y, 0);
	glTexCoord2f(1, 0); glVertex3f(x + w, y, 0);
	glTexCoord2f(1, 1); glVertex3f(x + w, y + h, 0);
	glTexCoord2f(0, 1); glVertex3f(x, y + h, 0);
	glEnd();
}

void DrawImageOffset(int texture_id, float x, float y) {
	int w = textures_width[texture_id];
	int h = textures_height[texture_id];
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[texture_id]);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(x, y, 0);
	glTexCoord2f(1, 0); glVertex3f(x + w, y, 0);
	glTexCoord2f(1, 1); glVertex3f(x + w, y + h, 0);
	glTexCoord2f(0, 1); glVertex3f(x, y + h, 0);
	glEnd();
}

void DrawSpriteRect(int sprite_id, float x, float y, float w, float h) {
	int texture_id = sprites_texture_ids[sprite_id];
	int tx = sprites_xs[sprite_id];
	int ty = sprites_ys[sprite_id];
	int tx2 = sprites_xs2[sprite_id];
	int ty2 = sprites_ys2[sprite_id];
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[texture_id]);
	
	glBegin(GL_QUADS);
	glTexCoord2f(tx, ty); glVertex3f(x, y, 0);
	glTexCoord2f(tx2, ty); glVertex3f(x + w, y, 0);
	glTexCoord2f(tx2, ty2); glVertex3f(x + w, y + h, 0);
	glTexCoord2f(tx, ty2); glVertex3f(x, y + h, 0);
	glEnd();
}

void DrawSpriteOffset(int sprite_id, float x, float y) {
	int texture_id = sprites_texture_ids[sprite_id];
	int w = textures_width[texture_id];
	int h = textures_height[texture_id];
	int tx = sprites_xs[sprite_id];
	int ty = sprites_ys[sprite_id];
	int tx2 = sprites_xs2[sprite_id];
	int ty2 = sprites_ys2[sprite_id];
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[texture_id]);
	
	glBegin(GL_QUADS);
	glTexCoord2f(tx, ty); glVertex3f(x, y, 0);
	glTexCoord2f(tx2, ty); glVertex3f(x + w, y, 0);
	glTexCoord2f(tx2, ty2); glVertex3f(x + w, y + h, 0);
	glTexCoord2f(tx, ty2); glVertex3f(x, y + h, 0);
	glEnd();
}

int AddSprite(int texture_id, float x, float y, float w, float h) {
	if (sprites_length >= SPRITES_CAP) {
		printf("Reached maximum number of sprites");
		exit(1);
	}
	
	int id = sprites_length;
	sprites_texture_ids[id] = texture_id;
	sprites_xs[id] = x;
	sprites_ys[id] = y;
	sprites_xs2[id] = x + w;
	sprites_ys2[id] = y + h;
	sprites_length++;
	return id;
}

void WriteFloatValue(FILE *f, float val) {
	// TODO: Consider endian when writing float value.
	// Don't know if this is necessary yet.
	fprintf(f, "%f : %x\n", val, *(int*)(void*)&val);
}

void WriteComment(FILE *f, char const *comment) {
	fprintf(f, "; %s\n", comment);
}

void WriteSpriteToFile(char *file, int sprite_id) {
	FILE *f;
	f = fopen(file, "w");
	fprintf(f, "%s", "Sprite\n");
	fprintf(f, "%d\n", sprite_id);
	WriteComment(f, "text");
	fprintf(f, "%d\n", sprites_texture_ids[sprite_id]);
	WriteComment(f, "coords");
	WriteFloatValue(f, sprites_xs[sprite_id]);
	WriteFloatValue(f, sprites_ys[sprite_id]);
	WriteFloatValue(f, sprites_xs2[sprite_id]);
	WriteFloatValue(f, sprites_ys2[sprite_id]);
	fclose(f);
}

void ReadIntField(FILE *f, int *arr, int id) {
	char line[LINE_CAP];
	while (fgets(line, LINE_CAP, f) != NULL) {
		if (line[0] == ';') continue;
		
		int val = 0;
		sscanf(line, "%d", &val);
		arr[id] = val;
		return;
	}
}

void ReadFloatField(FILE *f, float *arr, int id) {
	// TODO: Check for accurate binary representation.
	char line[LINE_CAP];
	while (fgets(line, LINE_CAP, f) != NULL) {
		if (line[0] == ';') continue;
		
		float val = 0;
		sscanf(line, "%f", &val);
		arr[id] = val;
		return;
	}
}

int ReadId(char *file, FILE *f, char *obj) {
	char line[LINE_CAP];
	fgets(line, LINE_CAP, f);
	int correct_object = 1;
	int i;
	for (i = 0; obj[i] != '\0' && line[i] != '\0'; i++) {
		if (obj[i] != line[i]) {
			correct_object = 0;
		}
	}
	
	if (!correct_object) {
		printf("Expected 'Sprite' at first line %s\n", file);
		exit(1);
	}
	
	int id;
	fgets(line, LINE_CAP, f);
	sscanf(line, "%d", &id);
	
	return id;
}

int ReadSpriteFromFile(char *file) {
	FILE *f;
	f = fopen(file, "r");
	int sprite_id = ReadId(file, f, "Sprite");
	float x = 0;
	float y = 0;
	float w = 0;
	float h = 0;
	
	ReadIntField(f, sprites_texture_ids, sprite_id);
	ReadFloatField(f, sprites_xs, sprite_id);
	ReadFloatField(f, sprites_ys, sprite_id);
	ReadFloatField(f, sprites_xs2, sprite_id);
	ReadFloatField(f, sprites_ys2, sprite_id);
	
	fclose(f);
	
	return sprite_id;
}

void Load(void)
{
	// Do loading code here.
	face = LoadTexture("face.png");
	
	// int testSprite = AddSprite(face, 0, 0, 1, 1);
	// WriteSpriteToFile("test.txt", testSprite);
	test_sprite = ReadSpriteFromFile("test.txt");
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	
	// DrawImageOffset(face, 0, 0);
	// DrawSpriteOffset(test_sprite, 0, 0);
	DrawSpriteRect(test_sprite, 0, 0, 100, 100);
	
    SDL_GL_SwapBuffers();
	
    if(SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}
	
    SDL_Flip(screen);
}

void SetOpenGLSettings(void) {
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,      8);
	
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,        32);
	
	SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    8);
	
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);
	
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);
	
	GLint viewport_x = 0;
	GLint viewport_y = 0;
	GLsizei viewport_width = 640;
	GLsizei viewport_height = 480;
	glViewport(viewport_x,
			   viewport_y,
			   viewport_width,
			   viewport_height);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0, 640, 480, 0, 1, -1);
	
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);
	
	glLoadIdentity();
}

int main( int argc, char* args[] ) {
	// Start SDL.
	SDL_Init( SDL_INIT_EVERYTHING );
	
	// Set up textures array.
	textures_length = 0;
	textures = malloc(sizeof(GLuint) * TEXTURES_CAP);
	textures_width = malloc(sizeof(int) * TEXTURES_CAP);
	textures_height = malloc(sizeof(int) * TEXTURES_CAP);
	
	// Set up sprites array.
	sprites_length = 0;
	sprites_texture_ids = malloc(sizeof(int) * SPRITES_CAP);
	sprites_xs = malloc(sizeof(int) * SPRITES_CAP);
	sprites_ys = malloc(sizeof(int) * SPRITES_CAP);
	sprites_xs2 = malloc(sizeof(int) * SPRITES_CAP);
	sprites_ys2 = malloc(sizeof(int) * SPRITES_CAP);
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;
	
	SDL_Surface *screen = NULL;
	screen = FULLSCREEN
	? SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_FULLSCREEN | SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL)
	: SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);
    if (!screen) {
        SDL_Quit();
        return 1;
    }
	
	SetOpenGLSettings();
	
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


