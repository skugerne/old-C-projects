#include "main.hpp"



shiptype::shiptype(shipnametype MODE, double X, double Y){
  mode = MODE;
  
  xCoordinate = X;
  yCoordinate = Y;
  xChange = 0;
  yChange = 0;
  angle = 0;
  
  collisionModifier = CATEGORY_SHIP;
  nameString = "Player Ship";
  
  basicInit();
  
  radius = 25;
  
  mass = 15;
  
  // pre-scaled by DT of 1/1000
  turnRate = .4;
  
  // pre-scaled by DT of 1/1000 (squared, in the case of accell)
  engineThrust = .00125;
  
  shieldGlow = 0;
  
  //----------------------------------------------------------------------
  // *** note ***
  // I am initializing various properties of this ship that are in fact ...
  // ... dependant on installed parts.  This is temporary.
  
  shldStrength = 500;
  shldMaxStrength = 500;
  shldRecharge = .002;   // 2 points per second I think
  
  weaponHeat[0] = 0;
  weaponMaxHeat[0] = 10000;
  weaponHeatUp[0] = 500;
  weaponCoolDown[0] = 2;      // scale for DT of 1/1000
  weaponLastFire[0] = 0;
  weaponReloadTime[0] = 50;
  
  weaponHeat[1] = 0;
  weaponMaxHeat[1] = 10000;
  weaponHeatUp[1] = 4000;
  weaponCoolDown[1] = 1;      // scale for DT of 1/1000
  weaponLastFire[1] = 0;
  weaponReloadTime[1] = 600;
  
  //----------------------------------------------------------------------
  
  falseifyControls();
}



void shiptype::draw(){
  glPushMatrix();
  
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
 
  glTranslatef(xShift , yShift , 0);
  glRotatef(angle , 0.0 , 0.0 , 1.0);

  glColor3f(1,0,0);
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
    drawLabel(true,1,1,1);
    drawHeading(true);
  
    #ifdef DEBUG_THRUST
    char buf[30];
  
    glEnable(GL_BLEND);
    glColor4f(1,0,1,0.85);
  
    snprintf(buf,30,"ahead:     %.2f",thrustStat1/DT);
    printStringToRight(FONT_SMALL,false,&buf[0],-25,125);
    snprintf(buf,30,"sideways:  %.2f",thrustStat2/DT);
    printStringToRight(FONT_SMALL,false,&buf[0],-25,105);
    snprintf(buf,30,"speed:     %.2f",thrustStat3/DT);
    printStringToRight(FONT_SMALL,false,&buf[0],-25,85);
   
    glDisable(GL_BLEND);
    #endif
  }
  
  glPopMatrix();
  
  //--------------------------------------------------------------------
  // note that this is a bad place to draw these output things, I think
  // over in playertype might be a better idea, but complex
  //--------------------------------------------------------------------
  
  // shield
  drawVerticalBar(490,0,300,20,shldStrength / shldMaxStrength,0.2,0.9,0.2);
  
  // weapon heat
  drawVerticalBar(-495,0,300,10,weaponHeat[0] / weaponMaxHeat[0],0.2,0.9,0.2);
  drawVerticalBar(-480,0,300,10,weaponHeat[1] / weaponMaxHeat[1],0.2,0.9,0.2);
}



objecttype* shiptype::specialUpdate(){

  //----------------------------------------------------------------------
  // engine and thrusters (inc creating flames)
  
  if(engineOn){
    engineGlow = 1.0;
    
    // max speed is pre-scaled by a DT of 1/1000
    thrust(angle, 2, engineThrust);
    
    // generating the flame effect
    if(_timestamp % 15 == _timestamp % 2 || _timestamp % 20 == _timestamp % 2){
      double tempA = angle * M_PI * 2 / 360;
      double particleX = xCoordinate - radius * cos(tempA);
      double particleY = yCoordinate - radius * sin(tempA);
      double particleDX, particleDY;
      
      // pre-scaled for DT of 1/1000
      double speedFactor = .5;
    
      particleDX = xChange - (speedFactor * cos(tempA+goof()/50));
      particleDY = yChange - (speedFactor * sin(tempA+goof()/50));
      createFlame(particleX,particleY,particleDX,particleDY);
      
      particleDX = xChange - (speedFactor * cos(tempA+goof()/25));
      particleDY = yChange - (speedFactor * sin(tempA+goof()/25));
      createFlame(particleX,particleY,particleDX,particleDY);
    }
  }
  
  //----------------------------------------------------------------------
  // turning things
  
  // our development e-brake
  if(turningLeft && turningRight){
    //xChange = 0;
    //yChange = 0;
    intoOrbit(xCoordinate,yCoordinate,xChange,yChange);
  }
  
  if(turningLeft){
    angle += turnRate;
  }
  if(turningRight){
    angle -= turnRate;
  }
  
  if(angle > 360) angle -= 360;
  else if(angle < 0) angle += 360;
  
  //----------------------------------------------------------------------
  // things cooling off and glowing down
  
  // fading of various glows, cooling of other stuff
  if(shieldGlow > 0){
    shieldGlow -= DT;
    if(shieldGlow < 0) shieldGlow = 0;
  }
  
  // shield recharge (not a permenent solution)
  if( shldStrength < shldMaxStrength ){
    shldStrength += shldRecharge;
    if( shldStrength > shldMaxStrength )
      shldStrength = shldMaxStrength;
  }
  
  // gun cooloff (not a permenent solution)
  for(int i=0;i<2;++i){
    if(weaponHeat[i] > 0){
      weaponHeat[i] -= weaponCoolDown[i];
      if(weaponHeat[i] < 0) weaponHeat[i] = 0;
    }
  }
  
  //----------------------------------------------------------------------
  
  #ifdef DEBUG_THRUST
  // keep our speed up to date (it is part of the truster output)
  thrustStat3 = sqrt(xChange * xChange + yChange * yChange);
  #endif
  
  #ifdef DEBUG_SHIPTYPE
  fprintf(stderr,"About to think about weapons.\n");
  #endif
  
  objecttype *oPtr = next;
  
  //----------------------------------------------------------------------
  // weapon firing
  
  if( weaponOn[0] && 
    weaponHeat[0] < weaponMaxHeat[0] && 
    weaponLastFire[0] + weaponReloadTime[0] <= _timestamp
  ){
    shotorigintype shotorigin = prepareShotOrigin(this,0,0,0,0,0);
    objecttype *sPtr = new shottype(shotorigin,0,SHOT_MED,SHOT_SINGLE,SHOT_NORM);
    sPtr->addToNewList();
    
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"Ship %u fired shot %u of type A.\n",idNum,sPtr->getID());
    #endif
    
    weaponLastFire[0] = _timestamp;
    weaponHeat[0] += weaponHeatUp[0];
  }
  
  if( weaponOn[1] && 
    weaponHeat[1] < weaponMaxHeat[1] && 
    weaponLastFire[1] + weaponReloadTime[1] <= _timestamp
  ){
    shotorigintype shotorigin = prepareShotOrigin(this,0,0,0,0,0);
    objecttype *sPtr = new shottype(shotorigin,0,SHOT_MED,SHOT_QUAD,SHOT_PROX);
    sPtr->addToNewList();
    
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"Ship %u fired shot %u of type B.\n",idNum,sPtr->getID());
    #endif
    
    weaponLastFire[1] = _timestamp;
    weaponHeat[1] += weaponHeatUp[1];
  }
  
  //----------------------------------------------------------------------
  
  return oPtr;
}



void shiptype::collisionEffect(double damage, objectcategorytype what){
  shldStrength -= damage;
  
  if(shldStrength < 0 && !isDead){
    this->destroy();
  }else{
    if(what == CATEGORY_DUST){
      shieldGlow += .1;
      if(shieldGlow > 1) shieldGlow = 1;
      else if(shieldGlow < .3) shieldGlow = .3;
    }else{
      shieldGlow = 1;
    }
    
    if(_playerShip == this){
      if(_player) _player->addShake(3*damage);
    }
  }
}



// make the object explode, mark as dead
void shiptype::destroy(){

  // we're dead
  isDead = true;
  
  // create some flame
  for(int i=0;i<200;++i){
    double speedFactor = 
      10.0 * (double)rand() / ( (double)RAND_MAX+1.0 );
    
    double tempA =
      2 * M_PI * ( (double)rand() / ( (double)RAND_MAX+1.0 ) );
  
    double particleDX, particleDY;
  
    particleDX = xChange - speedFactor * .3 * cos(tempA);
    particleDY = yChange - speedFactor * .3 * sin(tempA);
    createFlame(xCoordinate,yCoordinate,particleDX,particleDY);
  }

  // create some dust
  for(int i=0;i<150;++i)
    createDust(xCoordinate,yCoordinate,xChange,yChange,2);
}



// expects a string of known legth that dictates actions
void shiptype::provideInput(Uchar *data){
  engineOn = (bool)getIntValFromBitRange(data,0,0);
  turningLeft = (bool)getIntValFromBitRange(data,1,1);
  turningRight = (bool)getIntValFromBitRange(data,2,2);
  thrusterActivity = (thrusteractiontype)getIntValFromBitRange(data,3,5);
  //marker = getIntValFromBitRange(data,6,6);
  //message = getIntValFromBitRange(data,7,7);
  for(int i=0;i<MAX_SHIP_WEAPONS;++i)
    weaponOn[i] = (bool)getIntValFromBitRange(data,8+i,8+i);
  for(int i=0;i<MAX_SHIP_SPECIALS;++i)
    specialOn[i] = (bool)getIntValFromBitRange(data,8+i+MAX_SHIP_WEAPONS,8+i+MAX_SHIP_WEAPONS);
    
  #ifdef DEBUG_SHIPTYPE
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"Ship %d controls:\n",idNum);
  #else
  fprintf(stderr,"Ship controls (DEBUG_OBJECTTYPE for idNum):\n");
  #endif
  fprintf(stderr,"Engine: %d, Left: %d, Right: %d, Thruster: %d.\n",engineOn,turningLeft,turningRight,thrusterActivity);
  fprintf(stderr,"Weapons: ");
  for(int i=0;i<MAX_SHIP_WEAPONS;++i)
    fprintf(stderr,"[%d]",weaponOn[i]);
  fprintf(stderr,"\nSpecials: ");
  for(int i=0;i<MAX_SHIP_SPECIALS;++i)
    fprintf(stderr,"[%d]",specialOn[i]);
  fprintf(stderr,"\n");
  #endif
  
  // its up to this function to clean this up
  delete data;
}



void shiptype::falseifyControls(){
  engineOn = false;
  turningLeft = false;
  turningRight = false;
  thrusterActivity = THRUSTER_OFF;
  //marker = false;
  //message = false;
  for(int i=0;i<MAX_SHIP_WEAPONS;++i)
    weaponOn[i] = false;
  for(int i=0;i<MAX_SHIP_SPECIALS;++i)
    specialOn[i] = false;
}



double shiptype::getWarhead(objectcategorytype other){
  // some day add support for ramming shields here
  return 0.0;
}



void shiptype::drawShield(){
  if(shieldGlow <= 0) return;
  
  int sides = (int)radius;
  double aChangePer = M_PI * 2 / radius;
  
  glColor4f(0.0 , 0.0 , 0.0 , 0.0);
  
  glEnable(GL_BLEND);
  glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0 , 0.0);
      
    // select proper color for the shields to be
    glColor4f(0.0 , shieldGlow , 0.0 , 0.6 * shieldGlow);
      
    for(int i=0; i<sides+1; ++i){
      glVertex2f(
        radius * cos((double)i * aChangePer), 
        radius * sin((double)i * aChangePer)
      );
    }
  glEnd();
  glDisable(GL_BLEND);
}



void shiptype::drawVerticalBar(double xShift, double yShift, double hig, double wid, double per, double R, double G, double B){
  glPushMatrix();
  
  if( per > 1 ) per = 1;
  
  // if we're rotated, rotate us back
  if(_sceenRotation != 0.0){
    glRotatef(-_sceenRotation , 0.0 , 0.0 , 1.0);
  }
  
  glTranslatef(xShift,yShift,0);
  
  glColor3f(.1,.1,.1);
  glBegin(GL_QUADS);
    glVertex2f(-wid / 2, -hig / 2 + hig * per);
    glVertex2f(-wid / 2, hig / 2);
    glVertex2f(wid / 2, hig / 2);
    glVertex2f(wid / 2, -hig / 2 + hig * per);
  glEnd();
  glColor3f(R,G,B);
  glBegin(GL_QUADS);
    glVertex2f(-wid / 2, -hig / 2 + hig * per);
    glVertex2f(-wid / 2, -hig / 2);
    glVertex2f(wid / 2, -hig / 2);
    glVertex2f(wid / 2, -hig / 2 + hig * per);
    
  glEnd();
  glColor3f(1,1,1);
  glBegin(GL_LINE_LOOP);
    glVertex2f(-wid / 2, -hig / 2);
    glVertex2f(-wid / 2, hig / 2);
    glVertex2f(wid / 2, hig / 2);
    glVertex2f(wid / 2, -hig / 2);
  glEnd();
  
  glPopMatrix();
}
