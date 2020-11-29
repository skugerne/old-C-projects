#include "main.hpp"



alienrambotype::alienrambotype(double X, double Y){
  fprintf(stderr,"Rambo C!\n");
  
  xCoordinate = X;
  yCoordinate = Y;
  xChange = 0;
  yChange = 0;
  angle = 0;
  weaponAngle = 180;
 
  collisionModifier = COLLIDE_ALIEN;

  basicInit();

  radius = 40;

  mass = 30;

  // each pre-scaled for DT of 1/1000
  turnRate = .060;       // 60 deg/S
  enginePower = .0012;   // 1200 PPS^2
  maxSpeed = 1.5;        // 1500 PPS
  weaponTurnRate = 0.18; // 180 deg/S

  engineOn = false;
  turningRight = false;
  turningLeft = false;
  weaponOn = false;
  
  baseVisibility = 1;
  detectabilityFactor = 1 * radius;
  
  engineGlow = 0;
  weaponGlow = 0;
  
  weaponAngle = 90.0;
  weaponTurnRate = .180;
  weaponMaxAngle = 260.0;
  weaponMinAngle = 100.0;
  weaponLeft = false;
  weaponRight = false;
  weaponLastFired = 0;
  
  shieldPoints = 80;
  shieldGlow = 0;
}



void alienrambotype::draw(){
  glPushMatrix();
 
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
 
  glTranslatef(xShift , yShift , 0);
  glRotatef(angle , 0.0 , 0.0 , 1.0);
  
  //----------------------------------------------------------------------
  // draw the body
  
  glColor3f(1,1,0);
  
  glBegin(GL_LINE_STRIP);
    // front curve
    for(int i=-16;i<=16;++i){
      glVertex2f(
        (double)radius * cos( 2 * M_PI * ((double)i / 64) ),
        (double)radius * sin( 2 * M_PI * ((double)i / 64) )
      );
    }
  glEnd();
  
  glBegin(GL_LINES);
    // inner angles
    glVertex2i(0,35);
    glVertex2i(40,0);
    glVertex2i(40,0);
    glVertex2i(0,-35);
 
    // sideways line
    glVertex2i(0,-40);
    glVertex2i(0,40);
    
    // back line
    glVertex2i(-30,0);
    glVertex2i(40,0);
    
    // engine lines
    glVertex2i(0,35);
    glVertex2i(-10,35);
    glVertex2i(0,-35);
    glVertex2i(-10,-35);
  glEnd();
  
  //----------------------------------------------------------------------
  // draw the weapon (turret)
  
  glPushMatrix();
  glTranslatef(-30 , 0 , 0);
  glRotatef(weaponAngle , 0.0 , 0.0 , 1.0);
  glColor3f(1,0,0);
  
  glBegin(GL_LINES);
    // weapon lines
    glVertex2i(0,0);
    glVertex2i(100,0);
    glVertex2i(100,0);
    glVertex2i(80,20);
    glVertex2i(100,0);
    glVertex2i(80,-20);
  glEnd();
  
  glPopMatrix();
  
  //----------------------------------------------------------------------
  // draw misc stuff
  
  drawShield(); 
 
  if(_option1){
    drawRadius(true);
    drawLabel(false,.8,0,0);
  }
 
  glPopMatrix();
 
  #ifdef DEBUG_ALIEN_RAMBO
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



objecttype* alienrambotype::specialUpdate(){
  
  //----------------------------------------------------------------------
  // engine and thrusters (inc creating flames)
  
  if(engineOn){
    if(engineGlow < 1){
      engineGlow += DT * 2;
      if(engineGlow > 1) engineGlow = 1;
    }

    thrust(angle, maxSpeed, enginePower);
    
    if(_timestamp % 8 == 0){
      float tempA = angle * M_PI / 180;
      float particleX = xCoordinate - 10 * cos(tempA) + 35 * sin(tempA);
      float particleY = yCoordinate - 10 * sin(tempA) - 35 * cos(tempA);
      float particleDX, particleDY;
      
      // pre-scaled for DT of 1/1000
      float speedFactor = .35;
      
      particleDX = xChange - (speedFactor * cos(tempA+goof()/20));
      particleDY = yChange - (speedFactor * sin(tempA+goof()/20));
      createFlame(particleX,particleY,particleDX,particleDY);
    }
    
    if(_timestamp % 8 == 4){
      float tempA = angle * M_PI / 180;
      float particleX = xCoordinate - 10 * cos(tempA) - 35 * sin(tempA);
      float particleY = yCoordinate - 10 * sin(tempA) + 35 * cos(tempA);
      float particleDX, particleDY;
      
      // pre-scaled for DT of 1/1000
      float speedFactor = .35;
      
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
  
  if( turningLeft ){
    angle += turnRate;
  }
  if( turningRight ){
    angle -= turnRate;
  }
  if( weaponLeft ){
    weaponAngle += weaponTurnRate;
    if( weaponAngle > weaponMaxAngle )
      weaponAngle = weaponMaxAngle;
  }
  if( weaponRight ){
    weaponAngle -= weaponTurnRate;
    if( weaponAngle < weaponMinAngle )
      weaponAngle = weaponMinAngle;
  }

  if(angle > 360) angle -= 360;
  else if(angle < 0) angle += 360;

  aiupdate();
  
  objecttype *oPtr = next;
  
  //----------------------------------------------------------------------
  // weapon firing
  
  if(weaponOn && weaponGlow < 15 && _timestamp - weaponLastFired >= 50){
    ++weaponGlow;
    
    weaponLastFired = _timestamp;
    
    shotorigintype shotorigin = prepareShotOrigin(this,0,0,0,0,0);
    objecttype *oPtr = new shottype(shotorigin,weaponAngle+90,SHOT_WEAK,SHOT_QUAD,SHOT_LONG);
    oPtr->addToNewList();
  }
  
  //----------------------------------------------------------------------
  
  return oPtr;
}
 
 
 
void alienrambotype::collisionEffect(double damage, objectcollisiontype what){
  
  // only damaged by weapons and the star
  if(what == COLLIDE_WEAPON || what == COLLIDE_STAR){
    shieldPoints -= damage;
  }
  
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
 
 
 
void alienrambotype::aifollow(objecttype *target, double deadAngle, double engineAngle){
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
  if((targetAngle < engineAngle) || (targetAngle > 360 - engineAngle))
    engineOn = true;
  else
    engineOn = false;
}
 
 
 
void alienrambotype::aiupdate(){
  
  // setting our direction
  if( _playerShip ) aifollow(_playerShip, 5, 30);
  
  targettype targets[1];
  int numTargets = findTargetsAngle(targets,1,1500.0,10.0,170.0);
  
  // pointing turret at things and firing as appropriate
  if( numTargets ){
     if( weaponAngle > targets[0].bearing ){
       weaponRight = true;
       weaponLeft = false;
     }else{
       weaponRight = false;
       weaponLeft = true;
     }
     
     if( fabsf(targets[0].bearing - weaponAngle) < 30 )
       weaponOn = true;
     else
       weaponOn = false;
  }else{
    weaponRight = false;
    weaponLeft = false;
    weaponOn = false;
  }
}
