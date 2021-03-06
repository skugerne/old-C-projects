#include "main.h"
#include "font.h"
#include "object.h"



// turn off graphics, do pure engine test
bool g_test;

// sdl and graphics stuff
SDL_Surface *g_screen;
int g_videoFlags;

// framerate and update rate stuff
unsigned g_frames, g_updates, g_timestamp;
unsigned g_secCountStart;
float g_fps, g_ups;

// local globals
unsigned lastTimestamp;
bool isRunning;



void init(){
  // timestamp is incremented once per loop
  g_timestamp = 0;
  g_secCountStart = 0;
  
  // used primarily for the g_fps and g_ups count
  g_frames = 0;
  g_updates = 0;
  
  isRunning = true;
}



int delay(){
  unsigned time = SDL_GetTicks();
  
  // so we can know our fps
  if(time / 1000 >= g_secCountStart + 2){
    g_fps = (float)g_frames / (float)(time / 1000 - g_secCountStart);
    g_ups = (float)g_updates / (float)(time / 1000 - g_secCountStart);
    
    g_secCountStart = time / 1000;
    
    g_frames = 0;
    g_updates = 0;
  }
  
  // one iteration per millisecond
  int iterations = time - lastTimestamp;
  lastTimestamp = time;
  
  // potentially sacrifice some simulation speed to hit 60 FPS
  // avoid huge batches of simulation loops
  return iterations > 17 ? 17 : iterations;
}



bool running(){
  return isRunning;
}



void setRunning(bool temp){
  isRunning = temp;
}



int main(int argc, char *argv[]){
  g_test = ( argc > 1 && strcmp(argv[1],"--test") == 0 );
  
  if( !g_test ){
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) <  0 ){
      fprintf(stderr,"ERROR - Failed to initialize SDL: %s.\n", SDL_GetError());
      exit(-1);
    }
    atexit(SDL_Quit);
    
    if( TTF_Init() ){
      fprintf(stderr,"ERROR - Failed to init TTF: %s.\n",TTF_GetError());
      exit(-1);
    }
    atexit(TTF_Quit);
  }
  
  init();
  
  if( !g_test ){
    g_videoFlags  = SDL_OPENGL;
    g_videoFlags |= SDL_DOUBLEBUF;
    
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    g_screen = SDL_SetVideoMode(SCREEN_WID, SCREEN_HIG, 16, g_videoFlags);
    if( !g_screen ){
      fprintf(stderr,"ERROR - Failed to set GL video mode: %s.\n",SDL_GetError());
      exit(-1);
    }
  }
  
  if( !g_test ) initDraw();
  if( !g_test ) initText();
  initObjects();
  
  //=======================================================================
  // the main loop
  
  while(running()){
    if( g_test ){
      update(1000);
      if( g_timestamp >= 100000 ){
        printf("Reached %u runs, result sum is %X.\n",g_timestamp,statSum());
        setRunning(false);
      }
    }else{
      draw();
    
      // delay returns the proper number of iternations to do
      update( delay() );
      
      // it looks like events are queued up and not lost
      SDL_Event event;
      while( SDL_PollEvent( &event ) ){
        switch( event.type ){
          case SDL_KEYDOWN:
            handleKeyDown(&event.key);
            break;
          case SDL_QUIT:
            setRunning(false);
            break;
        }
      }
    }
  }
  
  // clean up the window
  exit(0);
}
