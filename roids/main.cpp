#include "main.hpp"



//============================================================================



// main game control ... local to this file
bool isRunning, isPaused;

// sector parameters
sectortype _sectors[NUM_SECTORS_PER_SIDE][NUM_SECTORS_PER_SIDE];

// window parameters
SDL_Surface *_screen;
int _videoFlags;
int _virtualPosX, _virtualPosY, _physicalX, _physicalY;
viewtype _viewMode;
double _sceenRotation;
double _goalSceenRotation;

double _fps, _ups;
Uint _timestamp;
Uint _frames, _updates;
Uint _time, _secCountStart;

// interface and visual parameters
bool _option1;
bool _option2;
bool _option3;
bool _option4;
Uint _fontDisLisStart, _numberOfFonts;
GLuint _texture[NUM_TEXTURES];

// object lists
objecttype* _objects;
objecttype *_deadObjects, *_newObjects;
starfieldtype *_stars[NUM_STARFIELDS];
startype *_starCore;
int _numObj;

// particle systems
particlesystemtype _flame, _dust;

// player stuff
double *_x, *_y, *_dx, *_dy, *_a;
shiptype *_playerShip, *_otherPlayerShip;
playertype *_player;
int _level;
Uint _curPlayers;
radartype _radar[NUM_SECTORS_PER_SIDE][NUM_SECTORS_PER_SIDE][2];
bool _radarNew;

// inputs
int _keys[NUM_KEYS];                       // keys that are pressed

// networking
SDLNet_SocketSet _sockSet;
TCPsocket _hostSocket, _otherSocket;  // the joiner never uses _hostSocket
gamemodetype _netStatus;
IPaddress _thisIP, _otherIP;



//============================================================================



bool running(){
  return isRunning;
}



void setRunning(bool temp){
  isRunning = temp;
}



bool paused(){
  return isPaused;
}



void setPaused(bool temp){
  isPaused = temp;
}



void init(){
  
  //=======================================================================
  // initialize...
  
  isRunning = true;
  isPaused = false;
  _netStatus = GAME_SOLO;
  
  // screen params
  _virtualPosX = 512;
  _virtualPosY = 382;
  _physicalX = 1024;
  _physicalY = 768;
  _viewMode = VIEW_WORLD_AXIS_ALIGNED;
  _sceenRotation = 0;
  _goalSceenRotation = 0;
  
  //=======================================================================
  
  _objects = NULL;
  _deadObjects = NULL;
  _newObjects = NULL;
  _playerShip = NULL;
  _otherPlayerShip = NULL;
  
  // the star core ... which we don't add to the list and which needs no resetting
  _starCore = new startype(MAX_COORDINATE/2,MAX_COORDINATE/2,0,0,25000000);
  
  resetForNewGame();
  
  initParticles();
  
  //=======================================================================
  // initialize the background star fields
  
  // watch out when you change the value of NUM_STARFIELDS
  
  // need to initialize starfields after _player because they look at ...
  // ... variables located in _player
  
  _stars[0] =  new starfieldtype(250,1.0, 1.0 );
  _stars[1] =  new starfieldtype(450,0.75,0.95);
  _stars[2] =  new starfieldtype(500,0.6 ,0.90);
  _stars[3] =  new starfieldtype(500,0.5 ,0.85);
  _stars[4] =  new starfieldtype(500,0.4 ,0.80);
  _stars[5] =  new starfieldtype(600,0.3 ,0.75);
  _stars[6] =  new starfieldtype(700,0.2 ,0.70);
  _stars[7] =  new starfieldtype(800,0.1 ,0.65);
  
  //=======================================================================
  // interface and visual parameters
  
  initMenu();
  _option1 = false;
  _option2 = false;
  _option3 = false;
  _option4 = true;
  
  //=======================================================================
  // clean the keys
  
  for(int i=0;i<NUM_KEYS;++i){
    _keys[i] = 0;
  }
  
  //=======================================================================
  // ready the network (mutexes, etc) for action
  
  //initNet();
}



int main(int argc, char *argv[]){
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0){
    fprintf(stderr,"ERROR - Failed to initialize SDL: %s.\n", SDL_GetError());
    exit(1);
  }
  
  if(SDLNet_Init() < 0){
    fprintf(stderr,"ERROR - Failed to initialize SDL_net: %s.\n", SDL_GetError());
    exit(2);
  }
  atexit(SDLNet_Quit);
  
  init(); 
  
  _videoFlags  = SDL_OPENGL;
  _videoFlags |= SDL_GL_DOUBLEBUFFER;
  
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );

  _screen = SDL_SetVideoMode(_physicalX, _physicalY, 16, _videoFlags);
  if(!_screen){
    fprintf(stderr,"ERROR - Failed to set GL video mode: %s.\n",SDL_GetError());
    SDL_Quit();
    exit(4);
  }
  
  #ifdef DEBUG_SDL
  fprintf(stderr,"We are past SDL_SetVideoMode.\n");
  #endif
  
  // intializes GL settings, some screen dimention stuff, builds fonts, etc
  initDraw();
  initUpdate();
  initNet();
  
  #ifdef DEBUG_UPDATE
  fprintf(stderr,"Draw and update are initialized.\n");
  #endif
  
  //=======================================================================
  // the main loop
  
  while(running()){
    #ifdef DEBUG_SDL
    fprintf(stderr,"Started main endless loop, about to update and draw.\n");
    #endif
    
    draw();
    
    // delay returns the proper number of iternations to do
    update( delay() );
    
    #ifdef DEBUG_SDL
    fprintf(stderr,"In main endless loop, about to check for input.\n");
    #endif
    
    // it looks like events are queued up and not lost
    SDL_Event event;
    while( SDL_PollEvent( &event ) ){
      /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
      switch( event.type ){
        case SDL_KEYDOWN:
          handleKeyDown(&event.key);
          break;
        case SDL_KEYUP:
          handleKeyUp(&event.key);
          break;
        case SDL_QUIT:
          setRunning(false);
          break;
      }
    }
    
    // input to playerShip
    if(_playerShip && _player)
      _playerShip->provideInput(_player->makeCommandString());
    
    // done with keyboard inputs, repeat loop
  }
  
  #ifdef DEBUG
  fprintf(stderr,"Quiting.\n");
  #endif
  
  // delete font display lists
  glDeleteLists( _fontDisLisStart, _numberOfFonts * 256 );
  
  // clean up our textures
  glDeleteTextures( NUM_TEXTURES, &_texture[0] );
  
  // clean up the window
  SDL_Quit();
  exit(0);
}
