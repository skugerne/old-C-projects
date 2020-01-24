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

//#define LONG_WAY
#define NUM_OBJ 1024
#define BASE_SIZE 1
#define OBJ_MIN_SIDES 4
#define WORLD_GRAV_ACC 0.00015

#define STAR_GLOW_1 2.0
#define STAR_GLOW_2 5.0
#define STAR_POINTS 64
#define STAR_SIDES 16
#define STAR_GRAVITY .005

#define WORLD_MIN_X 0
#define WORLD_MAX_X 800
#define WORLD_WID (WORLD_MAX_X-WORLD_MIN_X)
#define WORLD_MIN_Y 0
#define WORLD_MAX_Y 600
#define WORLD_HIG (WORLD_MAX_Y-WORLD_MIN_Y)

#define SECTOR_SIZE 16
#define NUM_SECTORS_WID (WORLD_WID/SECTOR_SIZE)
#define NUM_SECTORS_HIG (WORLD_HIG/SECTOR_SIZE)
#define UNDERFLOW_SECTOR 0
#define OVERFLOW_SECTOR_X (NUM_SECTORS_WID-1)
#define OVERFLOW_SECTOR_Y (NUM_SECTORS_HIG-1)
#define DRAW_OFFSET_X ((SCREEN_WID-(NUM_SECTORS_WID*SECTOR_SIZE))/2)
#define DRAW_OFFSET_Y ((SCREEN_HIG-(NUM_SECTORS_HIG*SECTOR_SIZE))/2)



extern object_t *g_objects;
extern sector_t g_sectors[NUM_SECTORS_WID][NUM_SECTORS_HIG];

// display stuff
extern bool g_showGrid, g_showCM;
extern unsigned g_colSearchCalls, g_collCalls;

// world options
extern bool g_worldGrav;
extern float g_worldGravDir;
extern bool g_gravStar;
extern bool g_elasticCollisions;
extern border_bounce_t g_borderType;



void initObjects();
void addSomeObjects(int);
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
