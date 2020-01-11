#ifndef MAIN_H
#define MAIN_H



#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include OPEN_GL       // defined in the Makefile
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>



//#define TTF_TEST_DEBUG



// sdl and graphics stuff
extern SDL_Surface *g_screen;
extern int g_videoFlags;

// framerate and update rate stuff
extern unsigned g_frames, g_updates, g_timestamp;
extern unsigned g_time, g_secCountStart;
extern float g_fps, g_ups;



// functions in main.c
bool running();
void setRunning(bool);

// functions in other.c
void initDraw();
void draw();
void update(unsigned);
void handleKeyDown(SDL_KeyboardEvent*);



#endif
