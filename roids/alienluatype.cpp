#include "main.hpp"



typedef int (alienluatype::*mem_func)();

// this template wraps a member function into a C-style "free" function compatible with lua
// https://stackoverflow.com/questions/32416388/how-to-register-member-function-to-lua-without-lua-bind-in-c
template <mem_func func>
int dispatch(lua_State * L) {
    alienluatype * ptr = *static_cast<alienluatype**>(lua_getextraspace(L));
    return ((*ptr).*func)();
}



int luaGlColor4f(lua_State *L){
  float r = luaL_checknumber(L, 1);
  float g = luaL_checknumber(L, 2);
  float b = luaL_checknumber(L, 3);
  float a = luaL_checknumber(L, 4);
  glColor4f(r,g,b,a);
  return 1;
}

int luaGlColor3f(lua_State *L){
  float r = luaL_checknumber(L, 1);
  float g = luaL_checknumber(L, 2);
  float b = luaL_checknumber(L, 3);
  glColor3f(r,g,b);
  return 1;
}

int luaGlBeginPolygon(lua_State *L){
  glBegin(GL_POLYGON);
  return 1;
}

int luaGlBeginLines(lua_State *L){
  glBegin(GL_LINES);
  return 1;
}

int luaGlVertex2i(lua_State *L){
  float x = luaL_checkinteger(L, 1);
  float y = luaL_checkinteger(L, 2);
  glVertex2i(x,y);
  return 1;
}

int luaGlEnd(lua_State *L){
  glEnd();
  return 1;
}

int luaGlEnableBlend(lua_State *L){
  glEnable(GL_BLEND);
  return 1;
}

int luaGlDisableBlend(lua_State *L){
  glDisable(GL_BLEND);
  return 1;
}



alienluatype::alienluatype(double X, double Y, const char *filename){
  xCoordinate = X;
  yCoordinate = Y;
  xChange = 0;
  yChange = 0;
  angle = 0;
 
  collisionModifier = CATEGORY_SHIP;
  nameString = "Lua Ship";

  basicInit();

  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"In alienluatype::alienluatype for idNum %u.\n",idNum);
  #endif

  fprintf(stderr,"Before starting Lua for %s.\n",filename);

  // fire up a Lua instance for this alien
  L = luaL_newstate();
  luaL_openlibs(L);
  *static_cast<alienluatype**>(lua_getextraspace(L)) = this;

  // register some C functions it can call back (basically specific calls to OpenGL)
  lua_pushcfunction(L, luaGlColor4f);
  lua_setglobal(L, "glColor4f");
  lua_pushcfunction(L, luaGlColor3f);
  lua_setglobal(L, "glColor3f");
  lua_pushcfunction(L, luaGlBeginPolygon);
  lua_setglobal(L, "glBeginPolygon");
  lua_pushcfunction(L, luaGlBeginLines);
  lua_setglobal(L, "glBeginLines");
  lua_pushcfunction(L, luaGlVertex2i);
  lua_setglobal(L, "glVertex2i");
  lua_pushcfunction(L, luaGlEnd);
  lua_setglobal(L, "glEnd");
  lua_pushcfunction(L, luaGlEnableBlend);
  lua_setglobal(L, "glEnableBlend");
  lua_pushcfunction(L, luaGlDisableBlend);
  lua_setglobal(L, "glDisableBlend");

  // register some member functions to call back
  lua_register(L, "findHot", &dispatch<&alienluatype::findHot>);
  lua_register(L, "setEngine", &dispatch<&alienluatype::setEngine>);
  lua_register(L, "setTurnLeft", &dispatch<&alienluatype::setTurnLeft>);
  lua_register(L, "setTurnRight", &dispatch<&alienluatype::setTurnRight>);
  lua_register(L, "setWeapon", &dispatch<&alienluatype::setWeapon>);
  lua_register(L, "setAiFlee", &dispatch<&alienluatype::setAiFlee>);
  lua_register(L, "setAiAttack", &dispatch<&alienluatype::setAiAttack>);
  lua_register(L, "setAiSearch", &dispatch<&alienluatype::setAiSearch>);

  // load the script
  if (luaL_loadfile(L, filename) == LUA_OK) {
      if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
          fprintf(stderr,"Seem to have have failed to load the Lua script (item 2).\n");
          exit(1);
      }
  }else{
      fprintf(stderr,"Seem to have have failed to load the Lua script (item 1).\n");
      exit(1);
  }

  if(lua_type(L,-1) != LUA_TNIL){
    fprintf(stderr,"The top of the stack has type (should have nothing): %d\n", lua_type(L,-1));
    exit(1);
  }

  init();
}



//alienluatype::~alienluatype(){
//  if( weapons ) delete weapons;
//}



void alienluatype::prepLuaTablePropery(Uint index, const char *prop){
  lua_pushinteger(L, index+1);
  lua_gettable(L, -2);
  lua_pushstring(L, prop);
  lua_gettable(L, -2);
  fprintf(stderr,"The top of the stack has type (hopefully string or number): %d\n", lua_type(L,-1));
}



void alienluatype::init(){

  // - - - - - LOAD WEAPON STUFF - - - - -

  lua_getglobal(L, "weapons");
  fprintf(stderr,"The top of the stack has type (hopefully table): %d\n", lua_type(L,-1));
  #ifdef LUA53
  lua_len(L, -1);
  numWeapons = lua_tonumber(L,-1);
  lua_pop(L, 1);
  #endif
  #ifdef LUA51
  numWeapons = lua_objlen(L, -1);
  #endif
  fprintf(stderr,"Number of weapons is %d.\n",numWeapons);
  lua_pop(L, 1);

  if(lua_type(L,-1) != LUA_TNIL){
    fprintf(stderr,"The top of the stack has type (should have nothing): %d\n", lua_type(L,-1));
    exit(1);
  }
  
  weapons = new weapontype[numWeapons];
  for(Uint i=0; i<numWeapons; ++i){
    weapons[i].glow = 0;
    weapons[i].on = false;
    weapons[i].lastFired = 0;

    lua_getglobal(L, "weapons");

    prepLuaTablePropery(i,"x");
    weapons[i].x = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"y");
    weapons[i].y = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"angle");
    weapons[i].angle = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"xOffset");
    weapons[i].xOffset = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"yOffset");
    weapons[i].yOffset = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"fireDelay");
    weapons[i].fireDelay = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"glowLimit");
    weapons[i].glowLimit = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"glowCoolRate");
    weapons[i].glowCoolRate = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"shotname");
    weapons[i].shotname = shotnametypeFromString(lua_tostring(L,-1));
    lua_pop(L, 2);

    prepLuaTablePropery(i,"shotcount");
    weapons[i].shotcount = shotcounttypeFromString(lua_tostring(L,-1));
    lua_pop(L, 2);

    prepLuaTablePropery(i,"shotmod");
    weapons[i].shotmod = shotmodtypeFromString(lua_tostring(L,-1));
    lua_pop(L, 3);
  }

  // - - - - - LOAD ENGINE STUFF - - - - -

  if(lua_type(L,-1) != LUA_TNIL){
    fprintf(stderr,"The top of the stack has type (should have nothing): %d\n", lua_type(L,-1));
    exit(1);
  }

  lua_getglobal(L, "engines");
  fprintf(stderr,"The top of the stack has type (hopefully table): %d\n", lua_type(L,-1));
  #ifdef LUA53
  lua_len(L, -1);
  numEngines = lua_tonumber(L,-1);
  lua_pop(L, 1);
  #endif
  #ifdef LUA51
  numEngines = lua_objlen(L, -1);
  #endif
  fprintf(stderr,"Number of engines is %d.\n",numEngines);
  lua_pop(L, 1);

  if(lua_type(L,-1) != LUA_TNIL){
    fprintf(stderr,"The top of the stack has type (should have nothing): %d\n", lua_type(L,-1));
    exit(1);
  }

  engines = new enginetype[numEngines];
  for(Uint i=0; i<numEngines; ++i){
    lua_getglobal(L, "engines");

    prepLuaTablePropery(i,"firingMod");
    engines[i].firingMod = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"firingPoint");
    engines[i].firingPoint = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"x");
    engines[i].x = lua_tonumber(L,-1);
    lua_pop(L, 2);

    prepLuaTablePropery(i,"y");
    engines[i].y = lua_tonumber(L,-1);
    lua_pop(L, 3);
  }

  // - - - - - LOAD OTHER PROPERTIES - - - - -

  lua_getglobal(L, "radius");
  radius = lua_tonumber(L,-1);
  fprintf(stderr,"Set radius to %d.\n",(int)radius);
  lua_pop(L, 1);
  lua_getglobal(L, "mass");
  mass = lua_tonumber(L,-1);
  lua_pop(L, 1);
  lua_getglobal(L, "turnRate");
  turnRate = lua_tonumber(L,-1);
  lua_pop(L, 1);
  lua_getglobal(L, "enginePower");
  enginePower = lua_tonumber(L,-1);
  lua_pop(L, 1);
  lua_getglobal(L, "maxSpeed");
  maxSpeed = lua_tonumber(L,-1);
  lua_pop(L, 1);
  lua_getglobal(L, "shieldPoints");
  shieldPoints = lua_tonumber(L,-1);
  lua_pop(L, 1);

  if(lua_type(L,-1) != LUA_TNIL){
    fprintf(stderr,"The top of the stack has type (should have nothing): %d\n", lua_type(L,-1));
    exit(1);
  }

  // - - - - - NON-LUA PROPERTIES - - - - -

  engineOn = false;
  turningRight = false;
  turningLeft = false;

  baseVisibility = 1;           // a measure of how much illumination makes the object visible (for example color)
  detectability = 1 * radius;   // how apparent it is to radar-like scanning

  engineGlow = 0;
  shieldGlow = 0;
}



#ifdef DEBUG_ALIEN_FIGHTER
void drawAiLabel(const char *txt, int tx, int ty, bool var){
  if(var) glColor4f(0.2,1.0,0.2,0.85); else glColor4f(0.9,0.1,0.0,0.85);
  printStringToLeft(FONT_SMALL,false,txt,tx,ty);
}
#endif



void alienluatype::draw(){
  glPushMatrix();
 
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
 
  glTranslatef(xShift , yShift , 0);
  glRotatef(angle , 0.0 , 0.0 , 1.0);
 
  // call out to Lua to draw our alien
  lua_getglobal(L, "draw");
  if (lua_isfunction(L, -1)) {  // -1 is the element just pushed, a function
    lua_pushnumber(L, _timestamp);   // push an argument to the func
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
      fprintf(stderr,"Failed to call the Lua draw function (item 2).\n");  // can be defective Lua code
      exit(1);
    }
  } else {
    fprintf(stderr,"Failed to call the Lua draw function (item 1).\n");
    exit(1);
  }
 
  drawShield(); 
 
  if(_option1){
    drawRadius(true);
    drawLabel(false,.8,0,0);
  }

  glPopMatrix();

  #ifdef DEBUG_ALIEN_FIGHTER
  int tx = xShift-5-radius;
  int ty = yShift+60;
  glEnable(GL_BLEND);

  drawAiLabel("Engine", tx, ty-=20, engineOn);
  drawAiLabel("Left", tx, ty-=20, turningLeft);
  drawAiLabel("Right", tx, ty-=20, turningRight);
  drawAiLabel("Flee", tx, ty-=20, aiFlee);
  drawAiLabel("Attack", tx, ty-=20, aiAttack);
  drawAiLabel("Search", tx, ty-=20, aiSearch);

  glDisable(GL_BLEND);
  #endif
}



objecttype* alienluatype::specialUpdate(){
  
  //----------------------------------------------------------------------
  // engine and thrusters (inc creating flames)
  
  if(engineOn){
    if(engineGlow < 1){
      engineGlow += DT * 2;
      if(engineGlow > 1) engineGlow = 1;
    }

    thrust(angle, maxSpeed, enginePower);
    
    for(Uint i=0; i<numEngines; ++i){
      if(_timestamp % engines[i].firingMod == engines[i].firingPoint){
        double tempA = angle * M_PI / 180;
        double particleX = xCoordinate + engines[i].x * cos(tempA) + engines[i].y * sin(tempA);
        double particleY = yCoordinate + engines[i].x * sin(tempA) - engines[i].y * cos(tempA);
        double particleDX, particleDY;
      
        // pre-scaled for DT of 1/1000
        double speedFactor = .35;
    
        particleDX = xChange - (speedFactor * cos(tempA+goof()/20));
        particleDY = yChange - (speedFactor * sin(tempA+goof()/20));
        createFlame(particleX,particleY,particleDX,particleDY);
      }
    }
  }
  
  //----------------------------------------------------------------------
  // things cooling off and glowing down
  
  if(engineGlow > 0){
    engineGlow -= DT / 2;
    if(engineGlow < 0) engineGlow = 0;
  }
  for(Uint i=0; i<numWeapons; ++i)
    if(weapons[i].glow > 0){
      weapons[i].glow -= weapons[i].glowCoolRate * DT;
      if(weapons[i].glow < 0) weapons[i].glow = 0;
    }
  if(shieldGlow > 0){
    shieldGlow -= DT;
    if(shieldGlow < 0) shieldGlow = 0;
  }
  
  // visibility issues relating to engine, weapon, shield, etc use
  baseVisibility = engineGlow * 20 + 1;
  
  //----------------------------------------------------------------------
  // turning, AI
  
  if(turningLeft){
    angle += turnRate;
  }
  if(turningRight){
    angle -= turnRate;
  }
  angle = angleLimit(angle);

  if( _timestamp % AI_UPDATE_DIVISOR == 0 )    // AI update on some physics updates
    aiupdate();

  objecttype *oPtr = next;
  
  //----------------------------------------------------------------------
  // weapon firing

  for(Uint i=0; i<numWeapons; ++i){
    if(weapons[i].on && weapons[i].glow < weapons[i].glowLimit && _timestamp - weapons[i].lastFired >= weapons[i].fireDelay){
      ++weapons[i].glow;
      weapons[i].lastFired = _timestamp;

      shotorigintype shotorigin = prepareShotOrigin(this,weapons[i].x,weapons[i].y,weapons[i].angle,weapons[i].xOffset,weapons[i].yOffset);
      objecttype *oPtr = new shottype(shotorigin,0,weapons[i].shotname,weapons[i].shotcount,weapons[i].shotmod);
      oPtr->addToNewList();
    }
  }
  
  //----------------------------------------------------------------------
  
  return oPtr;
}



void alienluatype::collisionEffect(double damage, objectcategorytype what){
  shieldPoints -= damage;
  
  if(shieldPoints < 0 && !isDead){
    // we're dead
    isDead = true;
    
    // create some flame
    for(Uint i=0;i<20;++i){
      double speedFactor = 
        (double)rand() / ( (double)RAND_MAX+1.0 );
      
      double tempA =
        2 * M_PI * ( (double)rand() / ( (double)RAND_MAX+1.0 ) );
    
      double particleDX, particleDY;
    
      particleDX = xChange - speedFactor * .3 * cos(tempA);
      particleDY = yChange - speedFactor * .3 * sin(tempA);
      createFlame(xCoordinate,yCoordinate,particleDX,particleDY);
    }
  
    // create some dust
    for(Uint i=0;i<15;++i)
      createDust(xCoordinate,yCoordinate,xChange,yChange,2);
  }else{
    if(what == CATEGORY_DUST){
      shieldGlow += .1;
      if(shieldGlow > 1) shieldGlow = 1;
      else if(shieldGlow < .3) shieldGlow = .3;
    }else{
      shieldGlow = 1;
    }
  }
}
 
 
 
void alienluatype::aifollow(objecttype *target, double deadAngle, double engineAngle){
  //target is the object that is being followed
  //deadAngle is half the forward angle, that if the target is in it,
  //    this object will not change turning direction. This  controls
  //    "jigglyness".
  //engineAngle is half the forward angle, that if the target is in it,
  //    this object will turn its engine on. Otherwise the engine is turned
  //    off.
  
  //find the angle between facing and goal
  double distX = xCoordinate - target->x();
  double distY = yCoordinate - target->y();
  float targetAngle = atan(distY/distX) / M_PI * 180 - angle + 180;
  if(distX < 0)
    targetAngle += 180;
  targetAngle = angleLimit(targetAngle);

  //handle turning to face target
  if((targetAngle > deadAngle) && (targetAngle < 180)){
    turningLeft = true;
    turningRight = false;
  }else if((targetAngle < 360 - deadAngle) && (targetAngle > 180)){
    turningLeft = false;
    turningRight = true;
  }
 
  //handle moving towards target
  if((targetAngle < engineAngle) || (targetAngle > 360 - engineAngle)){
    engineOn = true;
    if(fabsf(distX) < 700 && fabsf(distY) < 700)
      for(Uint i=0; i<numWeapons; ++i)
        weapons[i].on = true;
  }else{
    engineOn = false;
    for(Uint i=0; i<numWeapons; ++i)
      weapons[i].on = false;
  }
}



void alienluatype::aiupdate(){
  fprintf(stdout,"Do aiupdate() at %d.\n",SDL_GetTicks());

  // call out to Lua to manage the AI choices
  lua_getglobal(L, "aiUpdate");
  if (lua_isfunction(L, -1)) {
    // we need to send in a nested table (called "world" in at least the first Lua script)

    lua_createtable(L, 0, 4);               // table primed for a number of hash-indexed items

    lua_pushnumber(L, _timestamp);
    lua_setfield(L, -2, "timestamp");       // maybe the AI wants to know what the time is

    lua_pushnumber(L, MAX_COORDINATE);
    lua_setfield(L, -2, "maxCoordinate");   // the AI needs to know how big the map is

    // add standard object properties as a table
    asLuaTable(L);

    // add extra properties to self-table
    lua_pushnumber(L, M_PI * angle / 180);
    lua_setfield(L, -2, "angle");

    // attach table about self
    lua_setfield(L, -2, "myself");          // the AI needs to know where it is

    // a list of massive objects to be afraid of
    lua_createtable(L, 1, 0);               // table primed for a certain number of massive items in a list

    // FIXME: if there is multiple massive objects, this should be a loop
    lua_pushnumber(L, 1);                   // add at indexes, starting at 1
    _starCore->asLuaTable(L);
    lua_settable(L, -3);                    // note syntax for adding at an index

    lua_setfield(L, -2, "massiveObjects");  // the AI needs to know about the massive objects

    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
      fprintf(stderr,"Failed to call the Lua aiUpdate function (item 2).\n");  // can be defective Lua code
      exit(1);
    }
  } else {
    fprintf(stderr,"Failed to call the Lua aiUpdate function (item 1).\n");
    exit(1);
  }
}



int alienluatype::findHot(){
  // *** to be called from Lua ***

  // find suitable sectors based on the sensitivity thresholds passed from Lua
  float minVisibility = luaL_checknumber(L, 1);    // sensor
  float minDetectability = luaL_checknumber(L, 2); // scanner
  bool returnShips = lua_toboolean(L, 3);
  bool returnRocks = lua_toboolean(L, 4);
  bool returnShots = lua_toboolean(L, 5);
  bool returnGoodies = lua_toboolean(L, 6);

  // a hard-coded limit on the number of objects and sectors that will be returned
  // this is because probably its not interesting to track too many targets, so we save time by not trying
  #define MAX_RESULTS 10

  typedef struct { 
    int i;
    int j;
    float visibilityMultiple;
    float detectabilityMultiple;
  } sector_result_type;

  // lists are each one element longer than the max result size
  objecttype *object_results[MAX_RESULTS+1];
  sector_result_type sector_results[MAX_RESULTS+1];

  int objectCount = 0;    // things close enough to list separately
  int sectorCount = 0;    // things far enough away to be grouped into sectors
  for(int i=0;i<MAX_RESULTS;++i){
    object_results[i] = NULL;
    sector_results[i].visibilityMultiple = 0;
    sector_results[i].detectabilityMultiple = 0;
  }

  for(int i=0;i<NUM_SECTORS_PER_SIDE;++i){
    int distX = pow(i - xSectorIndex,2);
    for(int j=0;j<NUM_SECTORS_PER_SIDE;++j){
      if(abs(i-xSectorIndex) < SECTOR_VISION_RANGE && abs(j-ySectorIndex) < SECTOR_VISION_RANGE){
        // ==== OBJECTS ====

        if(_sectors[i][j].timestamp != _timestamp) continue;  // ignore stale sectors

        objecttype *oPtr = _sectors[i][j].first;
        while(oPtr){
          if(oPtr != this){  // ignore self
            // push down items
            int k=MAX_RESULTS;   // loop backwards, starting one element short of the length
            for(;k>0;--k){
              bool condA = object_results[k-1] == NULL || object_results[k-1]->getVisibility() < oPtr->getVisibility();
              bool condB = object_results[k-1] == NULL || object_results[k-1]->getDetectability() < oPtr->getDetectability();
              if(condA || condB){    // push down stuff worse than ourselves
                object_results[k] = object_results[k-1];  // note list is (MAX_RESULTS+1) long
              }else{
                break;  // stop pushing down because we found our spot
              }
            }

            // insert where we stopped
            // note the "extra" element in the list is thrown away, and we dump useless stuff there
            if(k < MAX_RESULTS){
              objectCount += 1;
              #ifdef DEBUG_ALIEN_FIGHTER
              fprintf(stdout,"Keep object as target at index %d (values %0.01f & %0.01f).\n",k,oPtr->getVisibility(),oPtr->getDetectability());
              #endif
              object_results[k] = oPtr;
            }else{
              #ifdef DEBUG_ALIEN_FIGHTER
              fprintf(stdout,"Drop object as target because its too faint (values %0.01f & %0.01f).\n",oPtr->getVisibility(),oPtr->getDetectability());
              #endif
            }
          }
          oPtr = oPtr->getSecMore();
        }

        // ==== END OBJECT SELECTION ====
      }else{
        // ==== SECTORS ====

        float invdist = 1.0 / (float)(distX + pow(j - ySectorIndex,2));
        float visibilityMultiple = _radar[i][j][!_radarNew].visibilitySum * invdist;
        float detectabilityMultiple = _radar[i][j][!_radarNew].detectabilitySum * invdist;
        if(visibilityMultiple > minVisibility || minDetectability > minDetectability){

          // push down items
          int k=MAX_RESULTS;   // loop backwards, starting one element short of the length
          for(;k>0;--k){
            bool condA = visibilityMultiple > minVisibility && sector_results[k-1].visibilityMultiple < visibilityMultiple;
            bool condB = detectabilityMultiple > minDetectability && sector_results[k-1].detectabilityMultiple < detectabilityMultiple;
            if(condA || condB){    // push down stuff worse than ourselves
              sector_results[k].i = sector_results[k-1].i;  // note list is (MAX_RESULTS+1) long
              sector_results[k].j = sector_results[k-1].j;
              sector_results[k].visibilityMultiple = sector_results[k-1].visibilityMultiple;
              sector_results[k].detectabilityMultiple = sector_results[k-1].detectabilityMultiple;
            }else{
              break;  // stop pushing down because we found our spot
            }
          }

          // insert where we stopped
          // note the "extra" element in the list is thrown away, and we dump useless stuff there
          if(k < MAX_RESULTS){
            sectorCount += 1;
            #ifdef DEBUG_ALIEN_FIGHTER
            fprintf(stdout,"Keep (%d,%d) as target at index %d (values %0.01f & %0.01f).\n",i,j,k,visibilityMultiple,detectabilityMultiple);
            #endif
            sector_results[k].i = i;
            sector_results[k].j = j;
            sector_results[k].visibilityMultiple = visibilityMultiple;
            sector_results[k].detectabilityMultiple = detectabilityMultiple;
          }else{
            #ifdef DEBUG_ALIEN_FIGHTER
            fprintf(stdout,"Drop (%d,%d) as target because its too faint (values %0.01f & %0.01f).\n",i,j,visibilityMultiple,detectabilityMultiple);
            #endif
          }
        }

        // ==== END SECTOR SELECTION ====
      }
    }
  }

  // limit these to the length of the results lists
  objectCount = objectCount < MAX_RESULTS ? objectCount : MAX_RESULTS;
  sectorCount = sectorCount < MAX_RESULTS ? sectorCount : MAX_RESULTS;

  #ifdef DEBUG_ALIEN_FIGHTER
  fprintf(stdout,"Return %d objects to Lua.\n",objectCount);
  for(int i=0;i<objectCount;++i){
    fprintf(
      stdout,
      "Return %s as target at index %d (values %0.01f & %0.01f).\n",
      object_results[i]->name(),
      i,
      object_results[i]->getVisibility(),
      object_results[i]->getDetectability()
    );
  }

  fprintf(stdout,"Return %d sectors to Lua.\n",sectorCount);
  for(int i=0;i<sectorCount;++i){
    fprintf(
      stdout,
      "Return (%d,%d) as target at index %d (values %0.01f & %0.01f).\n",
      sector_results[i].i,
      sector_results[i].j,
      i,
      sector_results[i].visibilityMultiple,
      sector_results[i].detectabilityMultiple
    );
  }

  int startstacksize = lua_gettop(L);
  #endif

  // we will create a tripple-nested table
  // objectCount: num
  // sectorCount: num
  // objects:
  //   1:
  //     { fields from objecttype::asLuaTable() }
  // sectors:
  //   1:
  //     x: coordinate of sector
  //     y: coordinate of sector
  //     dx: radius-weighted average xChange for whatever the sector contains
  //     dy: radius-weighted average yChange for whatever the sector contains
  //     detectabilityMultiple: sum of scanner signal from sector
  //     visibilityMultiple: sum of sensor signal from sector

  // creates a new empty table and pushes it onto the stack
  lua_createtable(L, 0, 4);               // prime for 4 hash-indexed items

  lua_pushnumber(L, objectCount);
  lua_setfield(L, -2, "objectCount");
  lua_pushnumber(L, sectorCount);
  lua_setfield(L, -2, "sectorCount");

  lua_createtable(L, objectCount, 0);     // expect 'objectCount' items in a list

  for(int i=0;i<objectCount;++i){
    #ifdef DEBUG_ALIEN_FIGHTER
    fprintf(stdout,"Append object %d.\n",i);
    #endif

    lua_pushnumber(L, i+1);               // add at indexes, starting at 1
    object_results[i]->asLuaTable(L);     // add a table describing the object
    lua_settable(L, -3);                  // put table into table, pop insertion index and table from stack
  }

  lua_setfield(L, -2, "objects");         // put the table under the key 'objects'

  lua_createtable(L, sectorCount, 0);     // expect 'sectorCount' items in a list

  for(int i=0;i<sectorCount;++i){
    #ifdef DEBUG_ALIEN_FIGHTER
    fprintf(stdout,"Append sector %d.\n",i);
    #endif

    lua_pushnumber(L, i+1);               // add at indexes, starting at 1

    lua_createtable(L, 0, 6);             // expect 6 hash-indexed items

    lua_pushnumber(L, sector_results[i].i * SECTOR_SIZE);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, sector_results[i].j * SECTOR_SIZE);
    lua_setfield(L, -2, "y");

    radartype *rPtr = _radar[sector_results[i].i][sector_results[i].j];
    lua_pushnumber(L, rPtr->weightedXChange / rPtr->radiusSum);
    lua_setfield(L, -2, "dx");
    lua_pushnumber(L, rPtr->weightedYChange / rPtr->radiusSum);
    lua_setfield(L, -2, "dy");

    lua_pushnumber(L, sector_results[i].detectabilityMultiple);
    lua_setfield(L, -2, "detectabilityMultiple");
    lua_pushnumber(L, sector_results[i].visibilityMultiple);
    lua_setfield(L, -2, "visibilityMultiple");

    lua_settable(L, -3);                  // put table into table, pop insertion index and table from stack
  }

  lua_setfield(L, -2, "sectors");         // attach the list of the sectors, pop that table off the stack

  #ifdef DEBUG_ALIEN_FIGHTER
  if(lua_type(L,-1) != LUA_TTABLE){
    fprintf(stderr,"The top of the stack (end of findHot) has type (should have table): %d\n", lua_type(L,-1));
    exit(1);
  }

  if(lua_gettop(L) != startstacksize+1){
    fprintf(stderr,"There is more than one new item on the stack (end of findHot) (expected %d, got %d).\n",startstacksize+1,lua_gettop(L));
    exit(1);
  }
  #endif

  return 1;
}



int alienluatype::setEngine(){
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"In alienluatype::setEngine for idNum %u.\n",idNum);  // check message is delivered to right object
  #endif
  engineOn = lua_toboolean(L, 1);
  return 1;
}



int alienluatype::setTurnLeft(){
  turningLeft = lua_toboolean(L, 1);
  return 1;
}



int alienluatype::setTurnRight(){
  turningRight = lua_toboolean(L, 1);
  return 1;
}



int alienluatype::setWeapon(){
  bool state = lua_toboolean(L, 1);
  Uint32 weapon = luaL_checknumber(L, 2);   // or luaL_tonumber() ?
  fprintf(stdout,"setWeapon %d %d\n",state,weapon);
  weapons[weapon].on = state;
  return 1;
}



int alienluatype::setAiFlee(){
  #ifdef DEBUG_ALIEN_FIGHTER
  aiFlee = lua_toboolean(L, 1);
  #endif
  return 1;
}



int alienluatype::setAiAttack(){
  #ifdef DEBUG_ALIEN_FIGHTER
  aiAttack = lua_toboolean(L, 1);
  #endif
  return 1;
}



int alienluatype::setAiSearch(){
  #ifdef DEBUG_ALIEN_FIGHTER
  aiSearch = lua_toboolean(L, 1);
  #endif
  return 1;
}
