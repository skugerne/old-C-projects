#ifndef MAIN_HH
#define MAIN_HH



// got tired of writing "unsigned" everywhere
#define Uchar unsigned char
#define Ushort unsigned short
#define Uint unsigned int
#define Ulong unsigned long



// this is to easily get around visual C's for loop variable ...
// ... "refdefinition" non-standards compliant bug
#ifdef WIN32
  #include <windows.h>
  #define for if(0) ; else for
  #define M_PI 3.1415927
  #define STACK_POP_BUG
  // see textureFont.cpp for more
#endif



// the debug things
//#define DEBUG
//#define DEBUG_UPDATE
//#define DEBUG_DRAW
//#define DEBUG_NET
//#define DEBUG_NET_TRAFF
//#define DEBUG_THRUST
//#define DEBUG_OBJECTTYPE
//#define DEBUG_OBJ_LISTS
//#define DEBUG_SHIPTYPE
#define DEBUG_ALIEN_FIGHTER
//#define DEBUG_ALIEN_RAMBO
//#define DEBUG_ALIEN_WANDER
//#define DEBUG_COLLISIONS
//#define DEBUG_SDL
//#define DEBUG_KEYBOARD
//#define DEBUG_PARTICLES



#include <SDL/SDL.h>
#include SDL_NET       // defined in the Makefile
#include OPEN_GL       // defined in the Makefile
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include <pthread.h>
typedef void *(*pthread_func_t)(void*);



//============================================================================



#define MAX_SHIP_SLOTS 19
#define MAX_SHIP_WEAPONS 4
#define MAX_SHIP_SPECIALS 6
#define MAX_SHIP_POWERSYS 5

#define DT 0.001                    // 1000 updates per second
#define AI_UPDATE_DIVISOR 250        // 100 control samples per second for AI or player

#define NUM_SECTORS_PER_SIDE     256   // note: changing this breaks radar
#define SECTOR_SIZE              512

#define MAX_COORDINATE           NUM_SECTORS_PER_SIDE*SECTOR_SIZE
#define MAX_SPEED                4

#define OVERLAY_X 1024
#define OVERLAY_HALF_X OVERLAY_X/2
#define OVERLAY_Y 768
#define OVERLAY_HALF_Y OVERLAY_Y/2

#define MAX_ASTERIOD_MASS 150
#define GRAVITY_CONST 100.0

#define NET_BUFFER_SIZE 20   // if this is too small data may be lost
#define NUM_KEYS 8
#define RADAR_SIZE 128    // note: tied to NUM_SECTORS_PER_SIDE

#define NUM_TEXTURES 21    // this will include the font texture (listed first)

#define NUM_STARFIELDS 8
#define NUM_FLAME 256
#define NUM_DUST 1024



//============================================================================



enum viewtype { VIEW_WORLD_AXIS_ALIGNED,
                VIEW_PLAYER_MOTION_ALIGNED,
                VIEW_SIDESCROLL_ALIGNED,
                VIEW_PLAYER_AXIS_ALIGNED};
                    
// 2 bits networking (4 possible values)                                     
enum shipnametype { SHIP_STEALTH,
                    SHIP_BATTLE,
                    SHIP_WIZZARD };
                    
enum objectcollisiontype { COLLIDE_STAR,
                           COLLIDE_ROID,
                           COLLIDE_SHIP,
                           COLLIDE_BASE,
                           COLLIDE_ALIEN,
                           COLLIDE_GOODIES,
                           COLLIDE_WEAPON,
                           COLLIDE_DEFLECT,
                           COLLIDE_FLAME,
                           COLLIDE_DUST };

enum partclasstype { PART_WEAPON,
                     PART_ENGINE,
                     PART_THRUSTER,
                     PART_SHIELD,
                     PART_REACTOR,
                     PART_SENSCAN,
                     PART_OTHER,
                     PART_EMPTY };
                    
// 10 bits networking (1024 possible values)
enum partnametype {  PART_WEAPON_LCAN,
                     PART_ENGINE_CHEM,
                     PART_THRUSTER_CHEM,
                     PART_THRUSTER_ION,
                     PART_SHIELD_SINGLE,
                     PART_SHIELD_MULTI,
                     PART_SHIELD_B_SINGLE,
                     PART_SHIELD_B_MULTI,
                     PART_REACTOR_HFUS,
                     PART_REACTOR_LFUS,
                     PART_EMPTY_BASIC };

// 3 bits networking
enum thrusteractiontype { THRUSTER_GO,
                          THRUSTER_STOP,
                          THRUSTER_L,
                          THRUSTER_R,
                          THRUSTER_OFF };
                           
enum fonttype { FONT_SMALL,
                FONT_MEDIUM };
                
enum resolutiontype { RES_640x480,
                      RES_800x600,
                      RES_1024x768 };
                      
enum gamemodetype { GAME_SOLO,
                    GAME_HOST,
                    GAME_JOIN };



//============================================================================
// class pre-declarations (all classes, hopefully)



// the root of it all
class objecttype;

class playertype;

class shottype;

class basetype;
class shiptype;
class alienfightertype;
class alienrambotype;
class alienwandertype;
class alienluatype;

class roidtype;

class starfieldtype;

class startype;



//============================================================================
// struct definitions


/**/
// rather than set all _sector elements to NULL all the time ...
// ... we just use the timestamp to know when elements are out of date
struct sectortype {
  objecttype *first;
  Uint timestamp;
  
  // the gravity map ... pps accel
  float xAccel, yAccel, brightness;
  bool nearCenter;
};



struct radartype {
  Uint timestamp;
  
  // totals for things in this sector
  double visibility, detectability;
};



struct particletype {
  float x, y, dx, dy;
  Uint msLeft;
};



struct particlesystemtype {
  particletype *particles;
  Uint validBegin, validEnd, count;
  objectcollisiontype what;
};



//============================================================================
// general globals



// sector parameters
extern sectortype _sectors[NUM_SECTORS_PER_SIDE][NUM_SECTORS_PER_SIDE];

// window parameters
extern SDL_Surface *_screen;
extern int _videoFlags;
extern int _virtualPosX, _virtualPosY, _physicalX, _physicalY;
extern viewtype _viewMode;
extern double _sceenRotation;
extern double _goalSceenRotation;

extern double _fps, _ups;
extern Uint _timestamp;
extern Uint _frames, _updates;
extern Uint _time, _secCountStart;

// interface and visual parameters
extern bool _option1;
extern bool _option2;
extern bool _option3;
extern bool _option4;
extern Uint _fontDisLisStart, _numberOfFonts;
extern GLuint _texture[NUM_TEXTURES];

// object lists
extern objecttype* _objects;
extern objecttype *_deadObjects, *_newObjects;
extern starfieldtype* _stars[NUM_STARFIELDS];
extern startype *_starCore;
extern int _numObj;

// particle systems
extern particlesystemtype _flame, _dust;

// player stuff
extern double *_x, *_y, *_dx, *_dy, *_a;
extern shiptype *_playerShip, *_otherPlayerShip;
extern playertype *_player;
extern int _level;
extern Uint _curPlayers;
extern radartype _radar[RADAR_SIZE][RADAR_SIZE][2];
extern bool _radarNew;

// inputs
extern int _keys[NUM_KEYS];                       // keys that are pressed

// networking
extern SDLNet_SocketSet _sockSet;
extern TCPsocket _hostSocket, _otherSocket;  // the joiner never uses _hostSocket
extern gamemodetype _netStatus;
extern IPaddress _thisIP, _otherIP;



// ==========================================================================
// general headers (yeah, it's a bit stupid to include them all)



#include "objecttype.hpp"

#include "playertype.hpp"

#include "shottype.hpp"

#include "shiptype.hpp"

#include "alientype.hpp"
#include "alienfightertype.hpp"
#include "alienrambotype.hpp"
#include "alienwandertype.hpp"
#include "alienluatype.hpp"

#include "starfieldtype.hpp"
#include "startype.hpp"
#include "roidtype.hpp"



//============================================================================
// general function pre-declarations

                  
                  
// in main.cpp
bool running();
void setRunning(bool);
bool paused();
void setPaused(bool);
                  
// in collision.cpp
void collision(objecttype*,objecttype*);
void collision(float,float,float*,float*,objecttype*,objectcollisiontype);

// in create.cpp
void resetForNewGame();
void semiResetForNewGame();
void recursiveDelete(objecttype*);
void intoOrbit(double,double,double&,double&);
void initAsteriods();
void initPlayer();
void initOtherPlayer(bool);
void createFlame(double,double,double,double);
void createDust(double,double,double,double,double);
void createAlienFighters();
void createAlienRambos();
void createAlienWanderers();
void createAlienLuas();

// in update.cpp
void initUpdate();
double goof();
void update(int);
int delay();

// in draw.cpp
void initDraw();
void initDraw(resolutiontype);
void draw();

// in input.cpp
void handleKeyDown(SDL_KeyboardEvent*);
void handleKeyUp(SDL_KeyboardEvent*);

// in menu.cpp
void initMenu();
void drawMenu();
void updateMenu();
bool keyToMenu(int);

// in textureFont.cpp
void setOverlayMode();
void setNormalMode();
void printStringToLeft(fonttype,bool,const char*,double,double);
void printStringToRight(fonttype,bool,const char*,double,double);
void printStringCentered(fonttype,bool,const char*,double,double);
void printCharToRight(fonttype,bool,char,double,double);
double stringXOverlay(fonttype,int);
double stringX(fonttype,char*);
double stringX(fonttype,int);
double stringY(fonttype,char*);
bool loadFontTexture();    // loads and processes into GL disp lists
bool loadTextures();

// in net.cpp
void initNet();
void networkStartupHost();
void networkStartupJoin();
void networkShutdown();
bool hostListen();
bool clientConnect(char*);
bool doGreetings();
bool doObjectTransfer(objecttype*);
bool doEndGreetings();
Uint pow2(Uint);
Uint getIntValFromBitRange(Uchar*,Uint,Uint);
Uchar* setBitRangeToIntVal(Uint,Uchar*,Uint,Uint);
void spellOutString(Uint,char*);

// in particles.cpp
void initParticles();
void updateFlame();
void drawFlame();
void updateDust();
void drawDust();


#endif
