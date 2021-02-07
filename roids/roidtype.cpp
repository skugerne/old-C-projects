#include "main.hpp"



// for over-the-network creation
roidtype::roidtype(double X, double Y, double DX, double DY, double M){
  xCoordinate = X;
  yCoordinate = Y;
  xChange = DX;
  yChange = DY;
  mass = M;
  
  collisionModifier = CATEGORY_ROCK;
  nameString = "Asteroid";
  
  setRandomRotation();
  
  basicInit();
  
  hitPoints = mass / 3;
  origHitPoints = hitPoints;
  deathByFade = 1;
   
  // find our mode (ignore return value)
  determineMode();
  
  // init based on the mode
  if(standardMode){
    initStandardMode();
  }else{
    initBezierMode();
  }

  baseVisibility = 1;           // a measure of how much illumination makes the object visible (for example color)
  detectability = 1 * radius;   // how apparent it is to radar-like scanning
}



// limit the lifespan of small asteroids, adjust visual representation of damage, rotate
objecttype* roidtype::specialUpdate(){

  // asteriods created too small die after a bit (by shrinking!)
  if(mass < 20){
    if(mass >= 3){
      mass -= .001;
      radius = mass / 3;
      scale = sqrt(mass)/3;   // shame to have to do this so needlessly...
    }
    if(mass <= 10){
      deathByFade -= .0004;
      if(deathByFade < 0) die(0);
    }
  }
  
  // damage viewable
  damageGlow -= .005;
  if(damageGlow < 0) damageGlow = 0;

  // rotation
  angle += aChange;
  if(angle > 360) angle -= 360;
  else if(angle < 0) angle += 360;
  
  return next;
}



void roidtype::draw(){
  if(standardMode)
    drawStandard();
  else
    drawBezier();
  
  showDamage();
  
  if(_option1){
    drawRadius(false);
    drawLabel(false,0,1,.5);
  }
}



void roidtype::collisionEffect(double damage, objectcategorytype what){

  if(what == CATEGORY_SHOT){
    hitPoints -= damage;

    if(hitPoints <= 0){
      if(mass < 5) die(hitPoints - damage);
      else breakUp();
    }

    // turn on the visual cue
    damageGlow = 2.0;
  }else if(what == CATEGORY_STAR){
    if(mass < 15){
      die(0);
    }else{
      hitPoints -= (hitPoints * 1.1);
      breakUp();
    }
  }
}



void roidtype::breakUp(){

  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"BREAK: Object #%u.\n",idNum);
  #endif

  objecttype *oPtr;
  
  // --------------------------------------------------------------
  // in this function I am using timestamps for a non-random random
  // --------------------------------------------------------------
  
  if(_timestamp % 9 == 0){
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.2,yChange+.1,mass/6);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.8,yChange+.1,mass/9);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange-.2,yChange-.5,mass/7);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange-.4,yChange-.6,mass/5);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange-.1,yChange-.2,mass/6);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.4,yChange+.6,mass/10);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
  }else if(_timestamp % 7 == 0){
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.2,yChange+.1,mass/6);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.8,yChange+.1,mass/3);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange-.2,yChange-.5,mass/4);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange-.4,yChange-.6,mass/7);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
  }else if(_timestamp % 2 == 0){
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.2,yChange+.6,mass/4);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.6,yChange+.1,mass/5);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange-.1,yChange-.1,mass/2);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
  }else{
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.2,yChange+.1,mass/6);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange+.8,yChange+.1,mass/9);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange-.2,yChange-.5,mass/4);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
    oPtr = new roidtype(xCoordinate,yCoordinate,xChange-.4,yChange-.6,mass/3);
    oPtr->addToNewList();
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"BREAK: New object #%u.\n",oPtr->getID());
    #endif
    
  }
  
  // this should ensure that the broken asteriod is removed from the world
  isDead = true;
    
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"BREAK: Done.\n");
  #endif
  
  int sectorX = (int)xCoordinate / SECTOR_SIZE;
  if(sectorX == NUM_SECTORS_PER_SIDE) sectorX = NUM_SECTORS_PER_SIDE - 1;
  int sectorY = (int)yCoordinate / SECTOR_SIZE;
  if(sectorY == NUM_SECTORS_PER_SIDE) sectorY = NUM_SECTORS_PER_SIDE - 1;
  
  // create debrii
  for(int i=0;i<(int)(sqrt(mass)/2);++i){
    createDust(xCoordinate,yCoordinate,xChange,yChange,0.5);
    createDust(xCoordinate,yCoordinate,xChange,yChange,0.8);
    createDust(xCoordinate,yCoordinate,xChange,yChange,0.9);
    createDust(xCoordinate,yCoordinate,xChange,yChange,1.0);
    createDust(xCoordinate,yCoordinate,xChange,yChange,1.6);
  }
  
  // the remaining chunk is not damaged
  damageGlow = 0;
}



void roidtype::die(float bang){
  isDead = true;
  
  // make a little debrii
  if(bang > 0){
    for(int i=0;i<2;++i){
      createDust(xCoordinate,yCoordinate,xChange,yChange,0.125*bang);
      createDust(xCoordinate,yCoordinate,xChange,yChange,0.25*bang);
      createDust(xCoordinate,yCoordinate,xChange,yChange,0.5*bang);
    }
  }
}



// shows players how badly hurt the asteriod is
// this could be done in some player-specific place, someday, with an added ...
// ... feature so that all asteriods damage-glow when they first enter view
void roidtype::showDamage(){
  if(damageGlow <= 0) return;
  
  // get the damage thing over the asteriod visually
  glPushMatrix();
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
  float bearing = atan( yShift / xShift );
  if(xShift > 0) bearing += M_PI;
  glTranslatef(xShift , yShift , 0);
  glRotatef( (bearing / M_PI * 180) , 0.0 , 0.0 , 1.0);
  
  // we are drawing something of a donut
  float innerRad = radius * 1.2;
  float outerRad = innerRad + 20;
  
  // how large is the arc that each color spans
  float grnSpan = hitPoints / origHitPoints * M_PI;
  float redSpan = M_PI - grnSpan;
  
  // how many elements will make up this arc
  int grnSides = (int)(radius * grnSpan / M_PI);
  int redSides = (int)(radius * redSpan / M_PI);
  
  // what is the change in angle per element
  float grnChangePer = grnSpan * 2 / (float)grnSides;
  float redChangePer = redSpan * 2 / (float)redSides;
  
  float myAngle;
  float myDamageGlow = damageGlow;
  if(myDamageGlow > 1) myDamageGlow = 1;
  
  glEnable(GL_BLEND);
  glBegin(GL_TRIANGLE_STRIP);
    
    // draw a donut
    myAngle = -redSpan;
    for(int i=0; i<=redSides; ++i){
      // the inside is clear
      glColor4f(0.05 * myDamageGlow , 0.0 , 0.0 , 0.03 * myDamageGlow);
      glVertex2f(
        innerRad * cos(myAngle), 
        innerRad * sin(myAngle)
      );
      
      // the outside glows
      glColor4f(0.5 * myDamageGlow , 0.0 , 0.0 , 0.3 * myDamageGlow);
      glVertex2f(
        outerRad * cos(myAngle), 
        outerRad * sin(myAngle)
      );
      
      myAngle += redChangePer;
    }
    
    // draw a donut
    myAngle = redSpan;
    for(int i=0; i<=grnSides; ++i){
      // the inside is clear
      glColor4f(0.0 , 0.05 * myDamageGlow , 0.0 , 0.03 * myDamageGlow);
      glVertex2f(
        innerRad * cos(myAngle), 
        innerRad * sin(myAngle)
      );
      
      // the outside glows
      glColor4f(0.0 , 0.5 * myDamageGlow , 0.0 , 0.3 * myDamageGlow);
      glVertex2f(
        outerRad * cos(myAngle), 
        outerRad * sin(myAngle)
      );
      
      myAngle += grnChangePer;
    }
    
  glEnd();
  glDisable(GL_BLEND);
  
  glPopMatrix();
}



void roidtype::setRandomRotation(){
  // pre-scaled by DT of 1/1000
  aChange =
    .25 * ( (float)rand() / ( (float)RAND_MAX+1.0 ) ) - .125;
    
  angle = 0;
}



// based on mass, determine if we should do standard or bezier
void roidtype::determineMode(){
  if( mass / MAX_ASTERIOD_MASS >= 0.2 )
    standardMode = false;
  else
    standardMode = true;
}



// a few general things
void roidtype::initStandardMode(){
  // radius varies a bit, is 6x at one point, not 5x
  radius = 4 * sqrt(mass);
  scale = sqrt(mass)/3;
}



// *** THIS IA A DRAWING FUNCTION ***
// this procedure draws the roid (asteroid) in standard mode
void roidtype::drawStandard(){
  glPushMatrix();
  
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
  
  glTranslatef(xShift , yShift , 0);
  glRotatef(angle , 0.0 , 0.0 , 1.0);
  glScalef(scale , scale , scale);
  
  glEnable(GL_BLEND);
  
  glColor4f(0.5 , 0.5 , 0.5, deathByFade);
  glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0 , 0.0);
    glColor4f(0.3 , 0.3 , 0.3, deathByFade);
    glVertex2f(5.0 , 10.0);
    glVertex2f(-5.0 , 8.0);
    glVertex2f(-10.0 , 6.0);
    glVertex2f(-10.0 , -7.0);
    glVertex2f(-5.0 , -10.0);
    glVertex2f(5.0 , -10.0);
    glVertex2f(10.0 , -5.0);
    glVertex2f(12.0 , 5.0);
    glVertex2f(5.0 , 10.0);
  glEnd();
  
  glDisable(GL_BLEND);
  
  glPopMatrix();
}



void roidtype::initBezierMode(){
  // first, set up the random asteriod shapes...
  cornersPerSide = (int)(sqrt(mass));
  corners = cornersPerSide * 6;
  
  float xDiff, yDiff, stretch;
  
  cPoints[0][0] = (10 * sin((0 + 2) * .34907)) + (-50.0 + (100.0*(float)rand()/((float)RAND_MAX+1.0)))/30.0;
  cPoints[0][1] = (10 * sin((0 - 2) * .34907)) + (-50.0 + (100.0*(float)rand()/((float)RAND_MAX+1.0)))/30.0;
  cPoints[18][0] = cPoints[0][0];
  cPoints[18][1] = cPoints[0][1];
  cPoints[1][0] = (10 * sin((1 + 2) * .34907)) + (-50.0 + (100.0*(float)rand()/((float)RAND_MAX+1.0)))/30.0;
  cPoints[1][1] = (10 * sin((1 - 2) * .34907)) + (-50.0 + (100.0*(float)rand()/((float)RAND_MAX+1.0)))/30.0;
  for(int i=2;i<17;++i){
    if(((i - 1) % 3) == 0){
      xDiff = cPoints[i-1][0] - cPoints[i-2][0];
      yDiff = cPoints[i-1][1] - cPoints[i-2][1];
      stretch = (20.0*(float)rand()/((float)RAND_MAX+1.0))/10.0;
      cPoints[i][0] = cPoints[i-1][0] + (xDiff * stretch);
      cPoints[i][1] = cPoints[i-1][1] + (yDiff * stretch);
    } else {
      cPoints[i][0] = (10 * sin((i + 2) * .34907)) + (-50.0 + (100.0*(float)rand()/((float)RAND_MAX+1.0)))/30.0;
      cPoints[i][1] = (10 * sin((i - 2) * .34907)) + (-50.0 + (100.0*(float)rand()/((float)RAND_MAX+1.0)))/30.0;
    }
  }
  xDiff = cPoints[0][0] - cPoints[1][0];
  yDiff = cPoints[0][1] - cPoints[1][1];
  stretch = (20.0*(float)rand()/((float)RAND_MAX+1.0))/10.0;
  cPoints[17][0] = cPoints[0][0] + (xDiff * stretch);
  cPoints[17][1] = cPoints[0][1] + (yDiff * stretch);
  
  calcCenteringShifts();
  
  //============================================================
  // done with asteriod shapes
  
  radius = 4 * sqrt(mass);
  
  calcScale();
  
  nameString = "Bez Roid";
  
  next = NULL;
  prev = NULL;
}


// *** THIS IA A DRAWING FUNCTION ***
// this procedure draws the roid (asteroid) when in bezier mode
void roidtype::drawBezier(){
  glPushMatrix();
  
  // this way of getting the right translation is crude ... must fix sometime
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
  
  glTranslatef(xShift , yShift , 0.0);
  glRotatef(angle , 0.0 , 0.0 , 1.0);
  glScalef(scale , scale , scale);
  
  int k=0;
  for(int j=0;j<6;++j){
    for(int i=0;i<cornersPerSide;++i){
      pPoints[k][0] = bezierValue(j,i,0) - xCenteringShift;
      pPoints[k][1] = bezierValue(j,i,1) - yCenteringShift;
      ++k;
    }
  }
  
  glColor3f(0.5 , 0.5 , 0.5);
  glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0 , 0.0);
    glColor3f(0.3 , 0.3 , 0.3);
    for(int i=0;i<corners;++i){
      glVertex2f(pPoints[i][0] , pPoints[i][1]);
    }
  glEnd();
  
  // control points for the spline, etc
  if(_option2){
    glEnable(GL_BLEND);
    glColor4f(1.0,1.0,0.0,0.5);
    glBegin(GL_LINE_LOOP);
      for(int i=0;i<18;++i){
        glVertex2f(cPoints[i][0] - xCenteringShift , cPoints[i][1] - yCenteringShift);
      } 
    glEnd();
    glColor4f(1.0,0.0,0.0,0.5);
    glBegin(GL_LINES);
      for(int i=0;i<18;++i){
        glVertex2f(cPoints[i][0] - xCenteringShift - 1 , cPoints[i][1] - yCenteringShift + 1);
        glVertex2f(cPoints[i][0] - xCenteringShift + 1 , cPoints[i][1] - yCenteringShift - 1);
        glVertex2f(cPoints[i][0] - xCenteringShift - 1 , cPoints[i][1] - yCenteringShift - 1);
        glVertex2f(cPoints[i][0] - xCenteringShift + 1 , cPoints[i][1] - yCenteringShift + 1);
      }
    glEnd();
    glDisable(GL_BLEND);
  }
  
  glPopMatrix();
}



// the splines do not start nicely centered around 0,0
void roidtype::calcCenteringShifts(){
  xCenteringShift = cPoints[0][0];
  yCenteringShift = cPoints[0][1];
  for(int i=1;i<18;++i){
    xCenteringShift += cPoints[i][0];
    yCenteringShift += cPoints[i][1];
  }
  xCenteringShift /= 18;
  yCenteringShift /= 18;
}



// scaling a bezier roid so it appears to fit in its radius can be tricky
void roidtype::calcScale(){
  float deviation = 0;
  
  for(int i=0;i<18;++i){
    deviation += fabsf( cPoints[i][0] - xCenteringShift );
    deviation += fabsf( cPoints[i][1] - yCenteringShift );
  }
  
  deviation /= 18;
  
  scale = sqrt( mass / ( .5 * deviation ) );
}



// calulates points on the bezier curve at intervals from 0 to 10, ...
// ... segment refers to just which curve is being dealt with ...
// ... all of the contol point information is stored in cPoints
float roidtype::bezierValue(int segment, int i, int xy){
  // segment should be 0-5
  segment *= 3;
  
  float time = ((float)(i)) / (cornersPerSide - 1);
  float time2 = time * time;
  float time3 = time2 * time;
  float oneMinusTime = 1 - time;
  float oneMinusTime2 = oneMinusTime * (1 - time);
  float oneMinusTime3 = oneMinusTime2 * (1 - time);
  
  float value = cPoints[segment][xy] * oneMinusTime3;
  value += cPoints[segment+1][xy] * 3 * time * oneMinusTime2;
  value += cPoints[segment+2][xy] * 3 * time2 * oneMinusTime;
  value += cPoints[segment+3][xy] * time3;
  
  return value;
}
 


float roidtype::convertXToLocal(float x, float y){
  // not exactly the fastest way to calc both localX and localY
  float tempAngle = (angle * -.0174533);
  float tempX = (x - *_x) / scale;
  float tempY = (y - *_y) / scale;
  
  return (tempX * cos(tempAngle) - tempY * sin(tempAngle));
}



float roidtype::convertYToLocal(float x, float y){
  // not exactly the fastest way to calc both localX and localY
  float tempAngle = (angle * -.0174533);
  float tempX = (x - *_x) / scale;
  float tempY = (y - *_y) / scale;
  
  return (tempY * cos(tempAngle) + tempX * sin(tempAngle));
}
