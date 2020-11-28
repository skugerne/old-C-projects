#include "main.hpp"



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



alienluatype::alienluatype(double X, double Y){
  xCoordinate = X;
  yCoordinate = Y;
  xChange = 0;
  yChange = 0;
  angle = 0;
 
  collisionModifier = COLLIDE_ALIEN;

  basicInit();

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

  // load the script
  if (luaL_loadfile(L, "lua_test.lua") == LUA_OK) {
      if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
          fprintf(stderr,"Seem to have have failed to load the Lua script (item 2).\n");
          exit(1);
      }
  }else{
      fprintf(stderr,"Seem to have have failed to load the Lua script (item 1).\n");
      exit(1);
  }

  if(lua_type(L,-1) != LUA_TNIL){
    fprintf(stderr,"The top of the stack has type: %d\n", lua_type(L,-1));
    exit(1);
  }

  init();
}



void alienluatype::init(){

  // load from the Lua script
  lua_getglobal(L, "radius");
  radius = lua_tonumber(L,-1);
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

  if(lua_type(L,-1) != LUA_TNIL){
    fprintf(stderr,"The top of the stack has type: %d\n", lua_type(L,-1));
    exit(1);
  }

  engineOn = false;
  turningRight = false;
  turningLeft = false;
  weaponOn = false;
  
  baseVisibility = 1;
  detectabilityFactor = 1 * radius;
  
  engineGlow = 0;
  weaponGlow = 0;
  weaponLastFired = 0;
  
  shieldPoints = 10;
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
      if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
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
    
    if(_timestamp % 12 == 0){
      double tempA = angle * M_PI / 180;
      double particleX = xCoordinate - radius * cos(tempA);
      double particleY = yCoordinate - radius * sin(tempA);
      double particleDX, particleDY;
      
      // pre-scaled for DT of 1/1000
      double speedFactor = .35;
    
      particleDX = xChange - (speedFactor * cos(tempA+goof()/20));
      particleDY = yChange - (speedFactor * sin(tempA+goof()/20));
      createFlame(particleX,particleY,particleDX,particleDY);
    }
  }
  
  //----------------------------------------------------------------------
  // things cooling off and glowing down
  
  if(engineGlow > 0){
    engineGlow -= DT / 2;
    if(engineGlow < 0) engineGlow = 0;
  }
  if(weaponGlow > 0){
    weaponGlow -= DT / 2;
    if(weaponGlow < 0) weaponGlow = 0;
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

  if(angle > 360) angle -= 360;
  else if(angle < 0) angle += 360;

  aiupdate();

  objecttype *oPtr = next;
  
  //----------------------------------------------------------------------
  // weapon firing
  
  if(weaponOn && weaponGlow < 30 && _timestamp - weaponLastFired >= 50){
    ++weaponGlow;
    
    weaponLastFired = _timestamp;
    
    objecttype *oPtr = new shottype(this,angle,SHOT_WEAK,SHOT_DOUBLE,SHOT_FAST_LONG);
    oPtr->addToNewList();
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
    for(int i=0;i<20;++i){
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
    for(int i=0;i<15;++i)
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
  double targetAngle = atan(distY/distX) / M_PI * 180 - angle + 180;
  if(distX < 0)
    targetAngle += 180;
  if(targetAngle >= 360.0)
    targetAngle -= 360.0;
  if(targetAngle < 0.0)
    targetAngle += 360.0;

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
      weaponOn = true;
  }else{
    engineOn = false;
    weaponOn = false;
  }
}

 
 
 
void alienluatype::aiupdate(){
  // setting our direction
  if( _playerShip ) aifollow(_playerShip, 30, 20);
}
