#include "main.hpp"



alienwandertype::alienwandertype(double X, double Y){
  xCoordinate = X;
  yCoordinate = Y;
  xChange = .001;
  yChange = .001;
  angle = 0;
 
  collisionModifier = COLLIDE_ALIEN;

  basicInit();

  radius = 20;

  mass = 8;

  heading = 0;

  // each pre-scaled for DT of 1/1000
  turnRate = .090;       // 90 deg/S
  enginePower = .00005;  // 50 PPS^2
  maxSpeed = 0.1;        // 100 PPS

  weaponOn[0] = false;
  weaponOn[1] = false;
  weaponOn[2] = false;
  
  baseVisibility = 100000;
  detectabilityFactor = 1 * radius;
  
  weaponGlow[0] = 0;
  weaponLastFired[0] = 0;
  weaponGlow[1] = 0;
  weaponLastFired[1] = 0;
  weaponGlow[2] = 0;
  weaponLastFired[2] = 0;
  
  shieldPoints = 30;
  shieldGlow = 0;
}



void alienwandertype::draw(){
  glPushMatrix();
 
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
 
  glTranslatef(xShift , yShift , 0);
  glRotatef(angle , 0.0 , 0.0 , 1.0);
  
  glColor3f(1,0,0);
  glBegin(GL_LINES);
    // gun turrets
    glVertex2f(0,0);
    glVertex2f(0,16);
    glVertex2f(0,0);
    glVertex2f(13.9,-8);
    glVertex2f(0,0);
    glVertex2f(-13.9,-8);
  glEnd();
      
  // circle params
  int sides = 24;
  double aChangePer = M_PI * 2 / sides;
  
  glColor3f(0,0,1);
  glBegin(GL_LINE_LOOP);  
    // circle
    for(int i=0; i<sides+1; ++i){
      glVertex2f(
        10 * cos((double)i * aChangePer), 
        10 * sin((double)i * aChangePer)
      );
    }
    //glVertex2f(10,0);
  glEnd();
 
  drawShield(); 
 
  if(_option1){
    drawRadius(true);
    drawLabel(false,.8,0,0);
  }
 
  glPopMatrix();
 
  #ifdef DEBUG_ALIEN_WANDER
  char buf[30];
  glEnable(GL_BLEND);
  
  glColor4f(1,.8,0,.85);
  if( weaponOn[0] ){
    sprintf(buf,"Weapon0");
    printStringToLeft(FONT_SMALL,false,&buf[0],xShift-20,yShift+30);
  }
  if( weaponOn[1] ){
    sprintf(buf,"Weapon1");
    printStringToLeft(FONT_SMALL,false,&buf[0],xShift-20,yShift+10);
  }
  if( weaponOn[2] ){
    sprintf(buf,"Weapon2");
    printStringToLeft(FONT_SMALL,false,&buf[0],xShift-20,yShift-10);
  }
  
  glColor4f(1,0,0,.85);
  switch (plan) {
    case WANDER_FROM_STAR:
      sprintf(buf,"Run away from star!");
      break;
    case WANDER_FROM_NEAR:
      sprintf(buf,"Run away from nearest object!");
      break;
    case WANDER_TO_NEAR:
      sprintf(buf,"Attack nearest object!");
      break;
    default:
      sprintf(buf,"I'm confused.");
      break;
  }
  printStringToLeft(FONT_SMALL,false,&buf[0],xShift-20,yShift-30);
  
  glColor4f(.5,1,0,.85);
  for(int i=0;i<4;++i){
    sprintf(buf,"Target%d: %0.2f",i,targetDistances[i]);
    printStringToLeft(FONT_SMALL,false,&buf[0],xShift+150,yShift-30+20*i);
  }
  
  glDisable(GL_BLEND);
  #endif
  
}



objecttype* alienwandertype::specialUpdate(){
  
  // always thrusts just whereever it wants to go
  thrust(heading, maxSpeed, enginePower);
  
  //----------------------------------------------------------------------
  // things cooling off and glowing down
  
  for(int i=0;i<3;++i)
    if(weaponGlow[i] > 0){
      weaponGlow[i] -= DT / 2;
      if(weaponGlow[i] < 0) weaponGlow[i] = 0;
    }
  
  if(shieldGlow > 0){
    shieldGlow -= DT;
    if(shieldGlow < 0) shieldGlow = 0;
  }
  
  //----------------------------------------------------------------------
  // turning, AI
  
  // always turn
  angle += turnRate;

  if(angle > 360) angle -= 360;

  aiupdate();

  objecttype *oPtr = next;
  
  //----------------------------------------------------------------------
  // weapon firing
  
  if(weaponOn[0] && weaponGlow[0] < 5 && _timestamp - weaponLastFired[0] >= 100){
    ++weaponGlow[0];
  
    weaponLastFired[0] = _timestamp;
    
    objecttype *oPtr = new shottype(this,angle,SHOT_MED,SHOT_DOUBLE,SHOT_NORM);
    oPtr->addToNewList();
  }
  if(weaponOn[1] && weaponGlow[1] < 5 && _timestamp - weaponLastFired[1] >= 100){
    ++weaponGlow[1];
  
    weaponLastFired[1] = _timestamp;
    
    objecttype *oPtr = new shottype(this,angle+120,SHOT_MED,SHOT_DOUBLE,SHOT_NORM);
    oPtr->addToNewList();
  }
  if(weaponOn[2] && weaponGlow[2] < 5 && _timestamp - weaponLastFired[2] >= 100){
    ++weaponGlow[2];
  
    weaponLastFired[2] = _timestamp;
    
    objecttype *oPtr = new shottype(this,angle+240,SHOT_MED,SHOT_DOUBLE,SHOT_NORM);
    oPtr->addToNewList();
  }
  
  //----------------------------------------------------------------------
  
  return oPtr;
}
 
 
 
void alienwandertype::collisionEffect(double damage, objectcollisiontype what){
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
 
 
 
void alienwandertype::aiupdate(){
  
  #ifdef DEBUG_ALIEN_WANDER
  for(int i=0;i<4;++i)
    targetDistances[i] = 0;
  #endif
  
  targettype targets[4];
  int numTargets = findTargetsApp(targets,4,3000.0,45.0);
  
  #ifdef DEBUG_ALIEN_WANDER
  for(int i=0;i<numTargets;++i)
    if( targets[i].distance >= NO_TARGET ){
      fprintf(stderr,"WANDER: Invalid target returned.\n");
      exit(1);
    }
  #endif
  
  // turn guns off
  weaponOn[0] = false;
  weaponOn[1] = false;
  weaponOn[2] = false;
  
  // turn on our guns if they are within 10deg of pointing at a target
  for(int i=0;i<numTargets;++i){
    #ifdef DEBUG_ALIEN_WANDER
    if( _timestamp % 1000 == 0 )
      fprintf(stderr,"WANDER: distance %.2f, bearing %.2f .\n",targets[i].distance,targets[i].bearing);
    targetDistances[i] = targets[i].distance;
    #endif
    
    double diff = targets[i].bearing - angle;
    if(diff < 0) diff += 360;
    if(diff >= 360) diff -= 360;
    
    if(diff < 10 || diff > 350) weaponOn[0] = true;
    if(diff < 130 && diff > 110) weaponOn[1] = true;
    if(diff < 250 && diff > 230) weaponOn[2] = true;
  }
  
  double distStarX = xCoordinate - MAX_COORDINATE/2;
  double distStarY = yCoordinate - MAX_COORDINATE/2;
  
  if( distStarX * distStarX + distStarY * distStarY > 32768 &&
    targets[0].distance < NO_TARGET
  ){
    if(targets[0].distance > 1000){
      // set our heading to the closest of them
      heading = targets[0].bearing;
      #ifdef DEBUG_ALIEN_WANDER
      if( _timestamp % 1000 == 0 )
        fprintf(stderr,"WANDER: Go to near object: %.2f.\n",heading);
      plan = WANDER_TO_NEAR;
      #endif
    }else{
      // set our heading away from the closest of them
      heading = targets[0].bearing - 180;
      #ifdef DEBUG_ALIEN_WANDER
      if( _timestamp % 1000 == 0 )
        fprintf(stderr,"WANDER: Run away from near object: %.2f.\n",heading);
      plan = WANDER_FROM_NEAR;
      #endif
    }
  }else{
    double bearing = atan( distStarY / distStarX );
    if(distStarX > 0) bearing += M_PI;
    heading = (bearing - M_PI) * (180 / M_PI);
    #ifdef DEBUG_ALIEN_WANDER
    if( _timestamp % 1000 == 0 )
      fprintf(stderr,"WANDER: Run away from star: %.2f.\n",heading);
    plan = WANDER_FROM_STAR;
    #endif
  }
}
