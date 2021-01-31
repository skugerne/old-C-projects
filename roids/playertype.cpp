#include "main.hpp"



playertype::playertype(){
  xCoordinate = MAX_COORDINATE / 2;
  yCoordinate = MAX_COORDINATE / 2;
  xChange = 0;
  yChange = 0;
  angle = 0;
  
  // point these globals at _player's location
  _x = &xCoordinate;
  _y = &yCoordinate;
  _dx = &xChange;
  _dy = &yChange;
  _a = &angle;
  
  makeTarget(NULL);
  
  targetDead = true;
  
  // set all the keys to the defaults
  
  engineKey[0] = 273;  // up arrow
  engineKey[1] = (int)'e';
  
  leftRotKey[0] = 276;  // left arrow
  leftRotKey[1] = (int)'w';
  
  rightRotKey[0] = 275;   // right arrow
  rightRotKey[1] = (int)'r';
  
  thrusterKey[THRUSTER_GO][0] = (int)'e';
  thrusterKey[THRUSTER_GO][1] = (int)'e';
  thrusterKey[THRUSTER_STOP][0] = 274;        // down arrow
  thrusterKey[THRUSTER_STOP][1] = (int)'d';
  thrusterKey[THRUSTER_L][0] = (int)'s';
  thrusterKey[THRUSTER_L][1] = (int)'w';
  thrusterKey[THRUSTER_R][0] = (int)'f';
  thrusterKey[THRUSTER_R][1] = (int)'r';
  
  for(int i=0;i<MAX_SHIP_WEAPONS;++i)
    weaponKey[i][1] = (int)' ';
    
  weaponKey[0][0] = (int)'z';
  weaponKey[1][0] = (int)'x';
  weaponKey[2][0] = (int)'c';
  weaponKey[3][0] = (int)'v';
  
  specialKey[0][0] = (int)'a';
  specialKey[0][1] = (int)'a';
  specialKey[1][0] = (int)'q';
  specialKey[1][1] = (int)'q';
  specialKey[2][0] = (int)'g';
  specialKey[2][1] = (int)'g';
  specialKey[3][0] = (int)'t';
  specialKey[3][1] = (int)'t';
  specialKey[4][0] = (int)'y';
  specialKey[4][1] = (int)'y';
  specialKey[5][0] = (int)'u';
  specialKey[5][1] = (int)'u';
  
  for(int i=0;i<MAX_SHIP_SPECIALS;++i)
    specialLastChanged[i] = 50;      // huh?
  
  markerKey[0] = (int)'h';
  markerKey[1] = (int)'h';
  markerLastChanged = 50;            // huh?
  
  messageKey[0] = 13;      // enter
  messageKey[1] = 13;      // enter
  messageLastChanged = 50;           // huh?
  
  shake = 0.0;
  shakeTimeShift = 0.0;
}



void playertype::makeTarget(shiptype *newTarget){
  target = newTarget;
  
  if(target){
    // update where we're at
    xCoordinate = target->x() + xShakeShift;
    yCoordinate = target->y() + yShakeShift;
    xChange = target->dx();
    yChange = target->dy();
    angle = target->a();
    
    targetDead = false;
  }else{
    // just switching it now?
    // record the time
    if(!targetDead){
      targetDeadAt = _time;
      
      // in case we somehow end up out of bounds
      while(xCoordinate >= MAX_COORDINATE) xCoordinate -= MAX_COORDINATE;
      while(xCoordinate < 0) xCoordinate += MAX_COORDINATE;
      while(yCoordinate >= MAX_COORDINATE) yCoordinate -= MAX_COORDINATE;
      while(yCoordinate < 0) yCoordinate += MAX_COORDINATE;
      
      // viewpoint not moving
      xChange = 0;
      yChange = 0;
    }
    
    targetDead = true;
  }
}



void playertype::draw(){
  if(target){
    #ifdef DEBUG_DRAW
    fprintf(stderr,"Draw player with target.\n");
    #endif
    
    // needs to have a rotation matching the screen
    if(_sceenRotation != 0.0){
      glPushMatrix();
      glRotatef(_sceenRotation , 0.0 , 0.0 , 1.0);
    }
    
    drawRing();
    drawRingArrows();
    drawRingStar();
    
    if(_sceenRotation != 0.0){
      glPopMatrix();
    }
    
    drawSensor();
    drawScanner();
    
  }else{
    #ifdef DEBUG_DRAW
    fprintf(stderr,"Draw player without target.\n");
    #endif
  
    glPushMatrix();
    
    glTranslatef(-90,-102,0);
    
    glColor3f(.2,.6,.2);
    glBegin(GL_QUADS);
      glVertex2f(0,0);
      glVertex2f(0,24);
      glVertex2f(180,24);
      glVertex2f(180,0);
    glEnd();
    
    glColor3f(1,1,1);
    glBegin(GL_LINE_LOOP);
      glVertex2f(0,0);
      glVertex2f(0,24);
      glVertex2f(180,24);
      glVertex2f(180,0);
    glEnd();
    
    glPopMatrix();
    
    char buf[20];
    glEnable(GL_BLEND);
    glColor4f(1,1,1,1);
  
    snprintf(buf,20,"Respawning... %d",3 - (_time - targetDeadAt) / 1000);
    printStringCentered(FONT_SMALL, false, &buf[0], 0, -98);
    
    glDisable(GL_BLEND);
  }
}



objecttype* playertype::specialUpdate(){
  doShake();
  
  // re-use of funtion to keep us "locked on" to our target if it exists
  makeTarget(target);
  
  // targetDead is *almost* the same thing as target==NULL
  if(targetDead){
    // wait a sec and start the player
    if(_time - targetDeadAt > 3000)
      initPlayer();
  }
  
  return NULL;
}



// makes string that holds commands, pass it to _playerShip
// recipient must know how long the string is
// recipient should delete the string
Uchar* playertype::makeCommandString(){
  bool engine = false;
  bool Lturn = false;
  bool Rturn = false;
  thrusteractiontype thrReturn;
  bool thr[4];
  thr[0] = false;
  thr[1] = false;
  thr[2] = false;
  thr[3] = false;
  bool wpn[MAX_SHIP_WEAPONS];
  for(int i=0;i<MAX_SHIP_WEAPONS;++i)
    wpn[i] = false;
  bool spc[MAX_SHIP_SPECIALS];
  for(int i=0;i<MAX_SHIP_SPECIALS;++i)
    spc[i] = false;
  bool marker = false;
  bool message = false;
  
  for(int i=0;i<NUM_KEYS;++i){
    if(_keys[i] != 0){
      
      if(_keys[i] == engineKey[0] || _keys[i] == engineKey[1])
        engine = true;
        
      if(_keys[i] == leftRotKey[0] || _keys[i] == leftRotKey[1])
        Lturn = true;
        
      if(_keys[i] == rightRotKey[0] || _keys[i] == rightRotKey[1])
        Rturn = true;
      
      if(_keys[i] == thrusterKey[0][0] || _keys[i] == thrusterKey[0][1])
        thr[0] = true;
        
      if(_keys[i] == thrusterKey[1][0] || _keys[i] == thrusterKey[1][1])
        thr[1] = true;
        
      if(_keys[i] == thrusterKey[2][0] || _keys[i] == thrusterKey[2][1])
        thr[2] = true;
        
      if(_keys[i] == thrusterKey[3][0] || _keys[i] == thrusterKey[3][1])
        thr[3] = true;
      
      for(int j=0;j<MAX_SHIP_WEAPONS;++j){
        if(_keys[i] == weaponKey[j][0] || _keys[i] == weaponKey[j][1])
          wpn[j] = true;
      }
      
      for(int j=0;j<MAX_SHIP_SPECIALS;++j){
        if(_keys[i] == specialKey[j][0] || _keys[i] == specialKey[j][1])
          spc[j] = true;
      }
      
      if(_keys[i] == markerKey[0] || _keys[i] == markerKey[1])
        marker = true;
        
      if(_keys[i] == messageKey[0] || _keys[i] == messageKey[1])
        message = true;
      
    }
  }
  
  if(thr[THRUSTER_STOP]) thrReturn = THRUSTER_STOP;
  else if(thr[THRUSTER_GO]) thrReturn = THRUSTER_GO;
  else if(thr[THRUSTER_L] && !thr[THRUSTER_R]) thrReturn = THRUSTER_L;
  else if(thr[THRUSTER_R] && !thr[THRUSTER_L]) thrReturn = THRUSTER_R;
  else thrReturn = THRUSTER_OFF;
  
  Uchar *data = new Uchar[3];
  
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  
  data = setBitRangeToIntVal((Uint)engine,data,0,0);
  data = setBitRangeToIntVal((Uint)Lturn,data,1,1);
  data = setBitRangeToIntVal((Uint)Rturn,data,2,2);
  data = setBitRangeToIntVal((Uint)thrReturn,data,3,5);
  data = setBitRangeToIntVal((Uint)marker,data,6,6);
  data = setBitRangeToIntVal((Uint)message,data,7,7);
  for(int i=0;i<MAX_SHIP_WEAPONS;++i)
    data = setBitRangeToIntVal((Uint)wpn[i],data,8+i,8+i);
  for(int i=0;i<MAX_SHIP_SPECIALS;++i)
    data = setBitRangeToIntVal((Uint)spc[i],data,8+i+MAX_SHIP_WEAPONS,8+i+MAX_SHIP_WEAPONS);
  // some bits unused
  
  // recipient must know how long the string is
  // recipient should delete the string
  return &data[0];
}



void playertype::setEngineKey(int i, int value){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::setEngineKey.");
    return;
  }
  #endif
  
  engineKey[i] = value;
}



void playertype::setLeftRotKey(int i, int value){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::setLeftRotKey.");
    return;
  }
  #endif
  
  leftRotKey[i] = value;
}



void playertype::setRightRotKey(int i, int value){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::setRightRotKey.");
    return;
  }
  #endif
  
  rightRotKey[i] = value;
}



void playertype::setThrusterKey(int j, int i, int value){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0 || j < 0 || j > 3){
    fprintf(stderr,"ERROR - i or j is/are out of range at playertype::setThrusterKey.");
    return;
  }
  #endif
  
  thrusterKey[j][i] = value;
}



void playertype::setWeaponKey(int j, int i, int value){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0 || j < 0 || j > 14){
    fprintf(stderr,"ERROR - i or j is/are out of range at playertype::setWeaponKey.");
    return;
  }
  #endif
  
  weaponKey[j][i] = value;
}



void playertype::setSpecialKey(int j, int i, int value){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0 || j < 0 || j > 5){
    fprintf(stderr,"ERROR - i or j is/are out of range at playertype::setSpecialKey.");
    return;
  }
  #endif
  
  specialKey[j][i] = value;
}



void playertype::setMarkerKey(int i, int value){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::setMarkerKey.");
    return;
  }
  #endif
  
  markerKey[i] = value;
}



void playertype::setMessageKey(int i, int value){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::setMessageKey.");
    return;
  }
  #endif
  
  messageKey[i] = value;
}



int playertype::getEngineKey(int i){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::getEngineKey.");
    return 0;
  }
  #endif
  
  return engineKey[i];
}



int playertype::getLeftRotKey(int i){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::getLeftRotKey.");
    return 0;
  }
  #endif
  
  return leftRotKey[i];
}



int playertype::getRightRotKey(int i){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::getRightRotKey.");
    return 0;
  }
  #endif
  
  return rightRotKey[i];
}



int playertype::getThrusterKey(int j, int i){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0 || j < 0 || j > 3){
    fprintf(stderr,"ERROR - i or j is/are out of range at playertype::getThrusterKey.");
    return 0;
  }
  #endif
  
  return thrusterKey[j][i];
}



int playertype::getWeaponKey(int j, int i){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0 || j < 0 || j > 14){
    fprintf(stderr,"ERROR - i or j is/are out of range at playertype::getWeaponKey.");
    return 0;
  }
  #endif
  
  return weaponKey[j][i];
}



int playertype::getSpecialKey(int j, int i){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0 || j < 0 || j > 5){
    fprintf(stderr,"ERROR - i or j is/are out of range at playertype::getSpecialKey.");
    return 0;
  }
  #endif
  
  return specialKey[j][i];
}



int playertype::getMarkerKey(int i){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::getMarkerKey.");
    return 0;
  }
  #endif
  
  return markerKey[i];
}



int playertype::getMessageKey(int i){
  #ifdef DEBUG_PLAYERTYPE
  if(i > 1 || i < 0){
    fprintf(stderr,"ERROR - i is out of range at playertype::getMessageKey.");
    return 0;
  }
  #endif
  
  return messageKey[i];
}



void playertype::drawRing(){
  glEnable(GL_BLEND);
  
  // inner half of ring
  glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<50;++i){
      glColor4f(.7,.7,0,.2);
      glVertex2f(
        (float)(_virtualPosY-5) * cos( 2 * M_PI * ((float)i / 50) ),
        (float)(_virtualPosY-5) * sin( 2 * M_PI * ((float)i / 50) )
      );
      glColor4f(.7,.7,0,0);
      glVertex2f(
        (float)(_virtualPosY-8) * cos( 2 * M_PI * ((float)i / 50) ),
        (float)(_virtualPosY-8) * sin( 2 * M_PI * ((float)i / 50) )
      );
    }
    glColor4f(.7,.7,0,.2);
    glVertex2f( (float)(_virtualPosY-5), 0 );
    glColor4f(.7,.7,0,0);
    glVertex2f( (float)(_virtualPosY-8), 0 );
  glEnd();
  
  // outer half of ring
  glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<50;++i){
      glColor4f(.7,.7,0,.2);
      glVertex2f(
        (float)(_virtualPosY-5) * cos( 2 * M_PI * ((float)i / 50) ),
        (float)(_virtualPosY-5) * sin( 2 * M_PI * ((float)i / 50) )
      );
      glColor4f(.7,.7,0,0);
      glVertex2f(
        (float)(_virtualPosY) * cos( 2 * M_PI * ((float)i / 50) ),
        (float)(_virtualPosY) * sin( 2 * M_PI * ((float)i / 50) )
      );
    }
    glColor4f(.7,.7,0,.2);
    glVertex2f( (float)(_virtualPosY-5), 0 );
    glColor4f(.7,.7,0,0);
    glVertex2f( (float)(_virtualPosY), 0 );
  glEnd();
  
  glDisable(GL_BLEND);
}



void playertype::drawRingArrows(){  
  // draw the indicator arrows and the incoming warnings
  int sectorX = (int)(*_x / SECTOR_SIZE);
  int sectorY = (int)(*_y / SECTOR_SIZE);
  for(int i=sectorX-30;i<sectorX+30;++i){
    if(i >= 0 && i < NUM_SECTORS_PER_SIDE){
      for(int j=sectorY-30;j<sectorY+30;++j){
        if(j >= 0 && j < NUM_SECTORS_PER_SIDE){
          //fprintf(stderr,"<%d,%d>",i,j);
          if(_sectors[i][j].timestamp == _timestamp-1){
            //fprintf(stderr,"<post>");
            // _timestamp - 1 cause _timestamp is incremented right after each update
            
            objecttype *oPtr = _sectors[i][j].first;
            while(oPtr){
              if(oPtr != _playerShip && oPtr != _starCore){
                // find the bearing of the object
                float distX = *_x - oPtr->x();
                float distY = *_y - oPtr->y();
                float bearing = atan( distY / distX );
                if(distX > 0) bearing += M_PI;
                
                float scale = _virtualPosY / sqrt(distX * distX + distY * distY);
                
                if(scale <= 1){
                  // don't draw if its inside the ring
                  
                  // make it easier to see
                  scale *= 3;
                  scale *= scale;
                  if(scale < .1) scale = .1;
                  
                  // find the relative heading of the object
                  float movX = *_dx - oPtr->dx();
                  float movY = *_dy - oPtr->dy();
                  float heading = atan( movY / movX ) * 360 / M_PI / 2;
                  if(movX > 0) heading += 180;
                
                  // ------------------------------------------------------------------------------
                  // draw indicator arrow
                  
                  glColor3f(1,1,1);
                  
                  glPushMatrix();
                  glTranslatef((_virtualPosY-5) * cos(bearing), (_virtualPosY-5) * sin(bearing), 0);
                  glRotatef(heading-90,0,0,1);
                  glScalef(scale,scale,1);
                  
                  glBegin(GL_LINES);
                    glVertex2f(0,4);
                    glVertex2f(-4,-4);
                    glVertex2f(0,4);
                    glVertex2f(4,-4);
                  glEnd();
                  
                  glPopMatrix();
                  
                  // ------------------------------------------------------------------------------
                  // calculate and draw incoming warning
                  
                  float angleDiff = M_PI + bearing - M_PI * heading / 180;
                  
                  // shouldn't have to ever add to the angle
                  while(angleDiff > M_PI) angleDiff -= 2*M_PI;
                  
                  glEnable(GL_BLEND);
                  // heading just about right at us
                  if( fabsf(angleDiff) < M_PI / 6 ){
                    glColor4f(1,0,0,scale);
                    glBegin(GL_LINES);
                      glVertex2f((_virtualPosY-5) * cos(bearing), (_virtualPosY-5) * sin(bearing));
                      glVertex2f(50 * cos(bearing), 50 * sin(bearing));
                    glEnd();
                  }
                  glDisable(GL_BLEND);
                }
              }
              oPtr = oPtr->getSecNext();
            }
          }
        }
      }
    }
  }
}



void playertype::drawRingStar(){
  
  // get bearing of the star
  float distX = *_x - _starCore->x();
  float distY = *_y - _starCore->y();
  float bearing = atan( distY / distX );
  if(distX > 0) bearing += M_PI;
  
  float scale = _virtualPosY / sqrt(distX * distX + distY * distY);
  
  // find the relative heading of the star
  float movX = *_dx - _starCore->dx();
  float movY = *_dy - _starCore->dy();
  float heading = atan( movY / movX ) * 360 / M_PI / 2;
  if(movX > 0) heading += 180;
  
  // if we are really close there isn't any point in telling the player that ...
  // ... the star is about it eat them alive
  if(scale > .35) return;
  
  // scale it relative to danger (or something)
  scale *= 3;
  scale *= scale;
  if(scale < .05) scale = .05;
  
  // ------------------------------------------------------------------------------
  // star icon
  
  glEnable(GL_BLEND);
  
  glPushMatrix();
  glTranslatef((_virtualPosY-5) * cos(bearing), (_virtualPosY-5) * sin(bearing), 0);
  glRotatef(heading-90,0,0,1);
  glScalef(scale,scale,1);
  
  float fade = (.35 - scale) / .1;
  if(fade > 1) fade = 1;
  glColor4f(1.0,1.0,0.0,0.25*fade);
  
  glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    for(int i=0;i<=16;++i){
      glVertex2f(
        75 * cos( 2 * M_PI * ((float)i / 16) ),
        75 * sin( 2 * M_PI * ((float)i / 16) )
      );
    }
  glEnd();
  
  glColor4f(1,1,0,fade);
  
  glBegin(GL_LINE_LOOP);
    for(int i=0;i<16;++i){
      glVertex2f(
        75 * cos( 2 * M_PI * ((float)i / 16) ),
        75 * sin( 2 * M_PI * ((float)i / 16) )
      );
    }
  glEnd();
  
  glBegin(GL_LINES);
    glVertex2f(0,25);
    glVertex2f(0,125);
  glEnd();
  
  glPopMatrix();
  
  // ------------------------------------------------------------------------------
  // calculate and draw incoming warning
                  
  float angleDiff = M_PI + bearing - M_PI * heading / 180;
                  
  // shouldn't have to ever add to the angle
  while(angleDiff > M_PI) angleDiff -= 2*M_PI;
                  
  // heading just about right at us
  if( fabsf(angleDiff) < M_PI / 6 ){
    glColor4f(1,1,0,scale);
    glBegin(GL_LINES);
      glVertex2f((_virtualPosY-5) * cos(bearing), (_virtualPosY-5) * sin(bearing));
      glVertex2f(50 * cos(bearing), 50 * sin(bearing));
    glEnd();
  }
  glDisable(GL_BLEND);

}



void standardRadarBits(float shipX, float shipY){
  glEnable(GL_BLEND);

  // the ship
  shipX /= (NUM_SECTORS_PER_SIDE / RADAR_SIZE);
  shipY /= (NUM_SECTORS_PER_SIDE / RADAR_SIZE);
  glColor4f(0.3, 1.0, 0.3, 1.0);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2i(shipX,shipY);
  glColor4f(0.3, 1.0, 0.3, 0.0);
  glVertex2i(shipX+2,shipY+2);
  glVertex2i(shipX-2,shipY+2);
  glVertex2i(shipX-2,shipY-2);
  glVertex2i(shipX+2,shipY-2);
  glVertex2i(shipX+2,shipY+2);
  glEnd();

  // FIXME: the star is hard-coded
  glColor4f(1.0, 1.0, 0.3, 1.0);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2i(RADAR_SIZE/2,RADAR_SIZE/2);
  glColor4f(1.0, 1.0, 0.3, 0.0);
  glVertex2i(RADAR_SIZE/2+3,RADAR_SIZE/2+5);
  glVertex2i(RADAR_SIZE/2-3,RADAR_SIZE/2+5);
  glVertex2i(RADAR_SIZE/2-5,RADAR_SIZE/2);
  glVertex2i(RADAR_SIZE/2-3,RADAR_SIZE/2-5);
  glVertex2i(RADAR_SIZE/2+3,RADAR_SIZE/2-5);
  glVertex2i(RADAR_SIZE/2+5,RADAR_SIZE/2);
  glVertex2i(RADAR_SIZE/2+3,RADAR_SIZE/2+5);
  glEnd();

  glDisable(GL_BLEND);

  // outer edge
  glColor3f(0.8,0.8,0.0);
  glBegin(GL_LINE_LOOP);
    glVertex2f(-1,-1);
    glVertex2f(RADAR_SIZE+1,-1);
    glVertex2f(RADAR_SIZE+1,RADAR_SIZE+1);
    glVertex2f(-1,RADAR_SIZE+1);
  glEnd();
}



void playertype::drawSensor(){
  
  // *********************************************************************
  // visibility (sensor), on right
  
  int shipX = (int)*_x / SECTOR_SIZE;
  int shipY = (int)*_y / SECTOR_SIZE;
  
  glPushMatrix();
  glTranslatef(_virtualPosX-15-RADAR_SIZE,-_virtualPosY+15,0);
  
  glColor3f(0,0,0);
  glBegin(GL_QUADS);
    glVertex2f(0,0);
    glVertex2f(RADAR_SIZE,0);
    glVertex2f(RADAR_SIZE,RADAR_SIZE);
    glVertex2f(0,RADAR_SIZE);
  glEnd();

  glBegin(GL_POINTS);

  // dots for all the objects you can see
  for(int i=0;i<RADAR_SIZE;++i){
    for(int j=0;j<RADAR_SIZE;++j){
      float visibility = 0.0;
      for(int i2=i*RADAR_DATA_DIVISOR;i2<i*RADAR_DATA_DIVISOR+RADAR_DATA_DIVISOR;++i2){
        // we'll need to translate from radar display size to radar data size
        // NOTE: assumption is that radar data size is the same as or larger than display size, and integer scalable
        int distX = i2 - shipX;
        distX *= distX;
        for(int j2=j*RADAR_DATA_DIVISOR;j2<j*RADAR_DATA_DIVISOR+RADAR_DATA_DIVISOR;++j2){
          if(_radar[i2][j2][!_radarNew].visibilitySum > 0){
            int distY = j2 - shipY;
            distY *= distY;
            visibility += _radar[i2][j2][!_radarNew].visibilitySum / (float)(distX + distY);
          }
        }
      }

      if(visibility > 0.01/SENSOR_BRIGHT_SCALE){
        float color = SENSOR_BRIGHT_SCALE * visibility;
        glColor3f(color,color,color);
        glVertex2i(i,j);
      }
    }
  }

  glEnd();

  standardRadarBits(shipX,shipY);

  glPopMatrix();
}



void playertype::drawScanner(){
  
  // *********************************************************************
  // detectability (scanner), on left

  int shipX = (int)*_x / SECTOR_SIZE;
  int shipY = (int)*_y / SECTOR_SIZE;

  glPushMatrix();
  glTranslatef(-_virtualPosX+15,-_virtualPosY+15,0);

  glColor3f(0,0,0);
  glBegin(GL_QUADS);
    glVertex2f(0,0);
    glVertex2f(RADAR_SIZE,0);
    glVertex2f(RADAR_SIZE,RADAR_SIZE);
    glVertex2f(0,RADAR_SIZE);
  glEnd();

  glBegin(GL_POINTS);

  // dots for all the objects you can see
  for(int i=0;i<RADAR_SIZE;++i){
    for(int j=0;j<RADAR_SIZE;++j){
      float detectability = 0.0;
      for(int i2=i*RADAR_DATA_DIVISOR;i2<i*RADAR_DATA_DIVISOR+RADAR_DATA_DIVISOR;++i2){
        // we'll need to translate from radar display size to radar data size
        // NOTE: assumption is that radar data size is the same as or larger than display size, and integer scalable
        int distX = i2 - shipX;
        distX *= distX;
        for(int j2=j*RADAR_DATA_DIVISOR;j2<j*RADAR_DATA_DIVISOR+RADAR_DATA_DIVISOR;++j2){
          if(_radar[i2][j2][!_radarNew].detectabilitySum > 0){
            int distY = j2 - shipY;
            distY *= distY;
            detectability += _radar[i2][j2][!_radarNew].detectabilitySum / (float)(distX + distY);
          }
        }
      }

      if(detectability > 0.01/SCANNER_BRIGHT_SCALE){
        float color = SCANNER_BRIGHT_SCALE * detectability;
        glColor3f(color,color,color);
        glVertex2i(i,j);
      }
    }
  }

  glEnd();

  standardRadarBits(shipX,shipY);

  glPopMatrix();

  // *********************************************************************
  // labels (for various things not just the scanner display)
  // some other bits of text are done over in draw.cpp, so not related to the player

  glEnable(GL_BLEND);
  glColor4f(0.7,0.7,0.0,0.65);

  printStringToRight(FONT_SMALL, false, "Scanner",  RADAR_SIZE/2 - _virtualPosX - 35 + 10, 20 + RADAR_SIZE - _virtualPosY);
  printStringToLeft( FONT_SMALL, false, "Sensor",   -RADAR_SIZE/2 + _virtualPosX + 30 - 10, 20 + RADAR_SIZE - _virtualPosY);
  printStringToLeft( FONT_SMALL, false, "Shield",   _virtualPosX,                          -170);
  printStringToLeft( FONT_SMALL, false, "Strength", _virtualPosX,                          -186);
  printStringToRight(FONT_SMALL, false, "Weapon",   -_virtualPosX,                         -170);
  printStringToRight(FONT_SMALL, false, "Heat",     -_virtualPosX,                         -186);

  glDisable(GL_BLEND);
}



void playertype::doShake(){
  if(shake > 1){
    //fprintf(stderr,"Shake %f %d\n",shake,_timestamp);
    
    if(_timestamp % 100 == 0) shake *= .6667;
    
    float t = ((float)_timestamp / 10) * M_PI;
    float angle = ((float)_timestamp / 10) * M_PI;
    float offset =  shake * sin(t+shakeTimeShift);
    
    xShakeShift = cos( angle ) * offset;
    yShakeShift = sin( angle ) * offset;
    
    //fprintf(stderr,"Shake %f xSO %f ySO %f\n",shake,xShakeShift,yShakeShift);
  }else{
    shake = 0.0;
    xShakeShift = 0.0;
    yShakeShift = 0.0;
  }
}
