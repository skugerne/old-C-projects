#include "main.hpp"



typedef int (alienluatype::*mem_func)(lua_State * L);

// this template wraps a member function into a C-style "free" function compatible with lua
// https://stackoverflow.com/questions/32416388/how-to-register-member-function-to-lua-without-lua-bind-in-c
template <mem_func func>
int dispatch(lua_State * L) {
    alienluatype * ptr = *static_cast<alienluatype**>(lua_getextraspace(L));
    return ((*ptr).*func)(L);
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
 
  collisionModifier = COLLIDE_ALIEN;

  basicInit();

  fprintf(stderr,"Before starting Lua for %s.\n",filename);

  // fire up a Lua instance for this alien
  L = luaL_newstate();
  luaL_openlibs(L);

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

  #ifdef DEBUG_OBJECTTYPE
  fprintf(stdout,"In alienluatype::alienluatype for ID %d.\n",idNum);
  #endif
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
  
  baseVisibility = 1;
  detectabilityFactor = 1 * radius;
  
  engineGlow = 0;
  shieldGlow = 0;
}



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
  char buf[20];
  glEnable(GL_BLEND);
  glColor4f(1,.5,0,.85);
  
  if(engineOn){
    sprintf(buf,"Engine");
    printStringToLeft(FONT_SMALL,false,&buf[0],xShift-20,yShift-30);
  }
  if(turningLeft){
    sprintf(buf,"Left");
    printStringToLeft(FONT_SMALL,false,&buf[0],xShift-20,yShift+10);
  }
  if(turningRight){
    sprintf(buf,"Right");
    printStringToLeft(FONT_SMALL,false,&buf[0],xShift-20,yShift-10);
  }
  
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

  if( _timestamp % AI_UPDATE_DIVISOR == 0 )
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

 
 
void alienluatype::collisionEffect(double damage, objectcollisiontype what){
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
    if(what == COLLIDE_DUST){
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

  // setting our direction (to be moved to Lua)
  //if( _playerShip ) aifollow(_playerShip, 30, 20);

  // call out to Lua to manage the AI choices
  lua_getglobal(L, "aiUpdate");
  if (lua_isfunction(L, -1)) {
    // we need to send in a nested table

    lua_createtable(L, 0, 3);               // NOTE: should match number of items

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
    lua_createtable(L, 1, 0);               // NOTE: should match number of items (a list)

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



int alienluatype::findHot(lua_State *L){
  float r = luaL_checknumber(L, 1);
  float g = luaL_checknumber(L, 2);   // or luaL_tonumber() ?
  float b = luaL_checknumber(L, 3);
  fprintf(stdout,"Got (%f) (%f) (%f).\n",r,g,b);
  return 1;
}



int alienluatype::setEngine(lua_State *L){
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stdout,"In alienluatype::setEngine for ID %d.\n",idNum);
  #endif
  engineOn = lua_toboolean(L, 1);
  fprintf(stdout,"engineOn %d\n",engineOn);
  return 1;
}



int alienluatype::setTurnLeft(lua_State *L){
  turningLeft = lua_toboolean(L, 1);
  fprintf(stdout,"turningLeft %d\n",turningLeft);
  return 1;
}



int alienluatype::setTurnRight(lua_State *L){
  turningRight = lua_toboolean(L, 1);
  fprintf(stdout,"turningRight %d\n",turningRight);
  return 1;
}
