#include "main.hpp"



alienfightertype::alienfightertype(double X, double Y){
  xCoordinate = X;
  yCoordinate = Y;
  xChange = 0;
  yChange = 0;
  angle = 0;
 
  collisionModifier = COLLIDE_ALIEN;

  basicInit();

  radius = 20;

  mass = 8;

  // each pre-scaled for DT of 1/1000
  turnRate = .075;       // 75 deg/S
  enginePower = .00125;  // 1250 PPS^2
  maxSpeed = 1.0;        // 1000 PPS

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



void alienfightertype::draw(){
  glPushMatrix();
 
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
 
  glTranslatef(xShift , yShift , 0);
  glRotatef(angle , 0.0 , 0.0 , 1.0);
 
  glColor3f(0,1,0);
  glBegin(GL_LINES);
    glVertex2i(12,0);
    glVertex2i(-2,-7);
    glVertex2i(12,0);
    glVertex2i(-2,7);
    glVertex2i(12,0);
    glVertex2i(-12,0);
 
    // sideways line
    glVertex2i(-2,-7);
    glVertex2i(-2,7);
  glEnd();
 
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



objecttype* alienfightertype::specialUpdate(){
  
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
  
  if(weaponOn && weaponGlow < 5 && _timestamp - weaponLastFired >= 100){
    ++weaponGlow;
    
    weaponLastFired = _timestamp;
    
    shotorigintype shotorigin = prepareShotOrigin(this,0,0,0,0,0);
    objecttype *oPtr = new shottype(shotorigin,0,SHOT_WEAK,SHOT_SINGLE,SHOT_NORM);
    oPtr->addToNewList();
  }
  
  //----------------------------------------------------------------------
  
  return oPtr;
}

 
 
void alienfightertype::collisionEffect(double damage, objectcollisiontype what){
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
 
 
 
void alienfightertype::aifollow(objecttype *target, double deadAngle, double engineAngle){
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

 
 
 
void alienfightertype::aiupdate(){
  // setting our direction
  if( _playerShip ) aifollow(_playerShip, 30, 20);
}
