#ifndef OBJECT_H
#define OBJECT_H



struct object_t {
  unsigned id;
  bool alive;
  
  float x,y;
  float dx,dy;
  float radius;
  unsigned inCollision;  // set equal to g_timestamp if in collision
  
  object_t *less;        // to node at lower coordinate
  object_t *more;        // to node at higher coordinate
  
  object_t *prev;        // for linking into a list
  object_t *next;        // for linking into a list
};

struct sector_t {
  unsigned timestamp;
  object_t *tree;
};

struct list_stats_t {
  unsigned timestamp;
  
  unsigned numObj;
  
  float kineticEng;
  float inertiaX;
  float inertiaY;
  float centerMassX;
  float centerMassY;
  float mass;
};

enum border_bounce_t {
  BORDER_BOUNCE,
  BORDER_WRAP,
  BORDER_OPEN
};


#define OBJECT_DEBUG

#define NUM_SECTORS 40
#define UNDERFLOW_SECTOR 0
#define OVERFLOW_SECTOR 39
#define SECTOR_SIZE 16

//#define LONG_WAY
#define NUM_OBJ 1024
#define BASE_SIZE 1
#define OBJ_SIDES 7
#define GRAV_BOT_ACC 0.00025

#define STAR_GLOW_1 2.0
#define STAR_GLOW_2 5.0
#define STAR_POINTS 64
#define STAR_SIDES 16
#define STAR_GRAVITY .005



extern object_t *g_objects;
extern sector_t g_sectors[NUM_SECTORS][NUM_SECTORS];

// display stuff
extern bool g_showGrid, g_showCM;
extern unsigned g_colSearchCalls, g_collCalls;

// world options
extern bool g_gravBot;
extern bool g_gravStar;
extern border_bounce_t g_borderType;



void initObjects();
void initObject_t(object_t*);
void sortObject_t(object_t*);
void radiusChange(float);
void speedChange(float);
void cancelMomentum();
void setStar();
void drawObject_t(object_t*);
void updateObject_t(object_t*);
void listStats(list_stats_t*);
unsigned statSum();
void engridObject_t(object_t*);



#endif
