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
#define SCREEN_WID 1024
#define SCREEN_HIG 768

#define COLUMN_WID 130
#define COLUMN_1_X 5
#define COLUMN_2_X (COLUMN_1_X + COLUMN_WID)
#define COLUMN_3_X (COLUMN_2_X + COLUMN_WID)
#define COLUMN_4_X (COLUMN_3_X + COLUMN_WID + 20)
#define COLUMN_5_X (COLUMN_4_X + COLUMN_WID - 20)
#define COLUMN_6_X (COLUMN_5_X + COLUMN_WID + 100)
#define TEXT_SCREEN_TOP (SCREEN_HIG - 8)



// turn off graphics, do pure engine test
extern bool g_test;

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
