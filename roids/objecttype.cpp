#include "main.hpp"



#ifdef DEBUG_OBJECTTYPE
Uint lastID = 0;
#endif

#define INDEX_UPDATE_SKIP 32



objecttype* objecttype::update(){
  
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"Update for #%u.\n",idNum);
  #endif
  
  double speedSqrd = xChange * xChange + yChange * yChange;
  
  if(speedSqrd > MAX_SPEED * MAX_SPEED){
    // the object is assumed to pick up on this at a latter time
    isDead = true;
    
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"Object #%u is dead due to speed of %f.\n",idNum,speedSqrd);
    #endif
  }else{
    xCoordinate += xChange;
    yCoordinate += yChange;
  }
  
  if( _timestamp % INDEX_UPDATE_SKIP == sectorUpdateWhen ){
    
    // map our coordinates to a sector
    xSectorIndex = ((int)xCoordinate / SECTOR_SIZE) % NUM_SECTORS_PER_SIDE;
    ySectorIndex = ((int)yCoordinate / SECTOR_SIZE) % NUM_SECTORS_PER_SIDE;
    
    // wall bounce
    collideWithEdges();
    
    visibilityFactor = 
      _sectors[xSectorIndex][ySectorIndex].brightness * baseVisibility / radius;
  }
  
  // some things aren't safe if we are somehow out of bounds
  setInBounds();
  if( inBounds ) gravitate();
  
  return next;
}



objecttype* objecttype::sectorUpdate(){

  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"Sector update for #%u.\n",idNum);
  #endif
  
  // some things aren't safe if we are somehow out of bounds
  if( inBounds ){
    
    // checks for collisions, calls fcn to actually put this in _sector[][]
    placeInSector();
    
    // use of sector/2 is not flexable if radar or map size changes
    addToRadar(&_radar[xSectorIndex/2][ySectorIndex/2][_radarNew]);
    
    if( isDead ){
      // woops, we got killed this update
      // (isDead is a member of objecttype usually set elsewhere)
      
      #ifdef DEBUG_OBJECTTYPE
      fprintf(stderr,"Setting deadNext, moving dead object to dead list.\n");
      #endif
      
      // deadNext holds the next in update list, we've been moved to dead list
      objecttype *deadNext = next;
      
      moveToDead();
      
      return deadNext;
    }
  }
  
  return next;
}



// distance from the "other" object
double objecttype::distanceFrom(objecttype *other){
  if(other == NULL){
    return MAX_COORDINATE;
  }
  
  double distX = xCoordinate - other->xCoordinate;
  double distY = yCoordinate - other->yCoordinate;
  
  double answer = sqrt( distX * distX + distY * distY );
  
  return answer;
}



// can this object be seen?
bool objecttype::viewable(double distance){
  // pretty ugly...
  // I belive that _virtualPosY * 3 / 2 will cover the viewable rectangle ...
  // ... no matter how it is rotated
  if( (int)(distance - radius) < _virtualPosY * 3 / 2 ) return true;
  return false;
}



// assuming that the _objects list is either NULL or largest-first sorted,
//   this function will add the object to the list in the correct place
// this objects's next and prev pointers are overwritten
void objecttype::addToList(){

  if(_objects){
    
    objecttype *leadPtr = _objects;
    if( leadPtr == NULL || leadPtr->radius < radius ){
      _objects = this;
      next = leadPtr;
      prev = NULL;
    }else{
      objecttype *followPtr = leadPtr;
      leadPtr = leadPtr->next;
      
      while( leadPtr && leadPtr->radius > radius ){
        followPtr = leadPtr;
        leadPtr = leadPtr->next;
      }
      
      followPtr->next = this;
      next = leadPtr;
      prev = followPtr;
    }
    
    if( next )
      next->prev = this;
    
  }else{
    // this objecttype is the only one in the list
    _objects = this;
    next = NULL;
    prev = NULL;
  }
  
  #ifdef DEBUG_OBJ_LISTS
  int number = 0;
  
  objecttype *vPtr = _objects;
  
  while(vPtr != NULL){
    ++number;
    vPtr = vPtr->getNext();
  }
  
  fprintf(stderr,"There are now %d objects in the list.\n",number);
  #endif
}



// snips the objecttype out of the _objects list
void objecttype::removeFromList(){

  if(_objects == this){
    _objects = next;
    if(_objects)
      _objects->prev = NULL;
    next = NULL;
    prev = NULL;
  }else{
    if(next != NULL)
      next->prev = prev;
    if(prev != NULL)
      prev->next = next;
    next = NULL;
    prev = NULL;  
  }
  
  #ifdef DEBUG_OBJ_LISTS
  int number = 0;
  
  objecttype *vPtr = _objects;
  
  while(vPtr != NULL){
    ++number;
    vPtr = vPtr->getNext();
  }
  
  fprintf(stderr,"There are now %d objects in the list.\n",number);
  #endif
}



// add object to the list where new objects hang out till the end of the update
//   round, so that they don't get added at funny places during the update
void objecttype::addToNewList(){
  next = _newObjects;
  _newObjects = this;
}



// better have saved "next" before calling this...
// the dead list is where things wait to be deleted (not in _sectors[][], etc)
void objecttype::moveToDead(){

  removeFromList();
  
  if(_deadObjects != NULL){
    next = _deadObjects;
    _deadObjects->prev = this;
    _deadObjects = this;
    prev = NULL;
  }else{
    // this objecttype is the only one in the list
    _deadObjects = this;
    next = NULL;
    prev = NULL;
  }
  
  #ifdef DEBUG_OBJ_LISTS
  int number = 0;
  objecttype *vPtr = _deadObjects;
  while(vPtr != NULL){
    ++number;
    vPtr = vPtr->getNext();
  }
  
  fprintf(stderr,"There are now %d objects in the dead list.\n",number);
  #endif
}



// init function called by constructions of derived classes
void objecttype::basicInit(){
  next = NULL;
  prev = NULL;
  sectorLess = NULL;
  sectorMore = NULL;
  
  // note that if xCoordinate/yCoordinate are not set to anything yet,
  //   sadness may result
  xSectorIndex = (int)xCoordinate / SECTOR_SIZE;
  ySectorIndex = (int)yCoordinate / SECTOR_SIZE;
  
  isDead = false;
  
  // select a random number between 0 and INDEX_UPDATE_SKIP which
  //   tells us when this object does certain updates
  sectorUpdateWhen = (unsigned int)((double)INDEX_UPDATE_SKIP * (double)rand() / ( (double)RAND_MAX+1.0 ));
  
  #ifdef DEBUG_OBJECTTYPE
  idNum = lastID;
  ++lastID;
  #endif
  
  #ifdef DEBUG_THRUST
  thrustStat1 = 0;
  thrustStat2 = 0;
  thrustStat3 = 0;
  #endif
}



void objecttype::addToSector(sectortype *sPtr){
  sectorLess = NULL;
  sectorMore = NULL;
  
  if( sPtr->timestamp != _timestamp ){
    // the sector has stale data in it (overwrite it)
    sPtr->first = this;
    
    // mark the sector as having something in it
    sPtr->timestamp = _timestamp;
    
  }else{
    objecttype *nodePtr = sPtr->first;
    while( 1 ){
      if( nodePtr->xCoordinate > xCoordinate ){
        if( nodePtr->sectorMore ){
          nodePtr = nodePtr->sectorMore;
        }else{
          nodePtr->sectorMore = this;
          break;
        }
      }else{
        if( nodePtr->sectorLess ){
          nodePtr = nodePtr->sectorLess;
        }else{
          nodePtr->sectorLess = this;
          break;
        }
      }
    }
  }
}



void objecttype::addToRadar(radartype *rPtr){
  // if timestamps don't match, overwrite
  if(rPtr->timestamp != _timestamp){
    rPtr->timestamp = _timestamp;
    
    rPtr->visibility = visibilityFactor;
    rPtr->detectability = detectabilityFactor;
  }else{
    rPtr->visibility += visibilityFactor;
    rPtr->detectability += detectabilityFactor;
  }
}



// a debugging tool
void objecttype::drawRadius(bool alreadyTranslated){
  if(!alreadyTranslated){
    glPushMatrix();
    
    float xShift = xCoordinate - *_x;
    float yShift = yCoordinate - *_y;
  
    glTranslatef(xShift , yShift , 0.0);
  }
  
  glColor3f(0,1,0);
  glBegin(GL_LINE_LOOP);
    for(int i=0;i<60;++i){
      glVertex2f(
        (float)radius * cos( 2 * M_PI * ((float)i / 60) ),
        (float)radius * sin( 2 * M_PI * ((float)i / 60) )
      );
    }
  glEnd();
  
  if(!alreadyTranslated){
    glPopMatrix();
  }
}



// a debugging tool
void objecttype::drawLabel(bool alreadyTranslated, double R, double G, double B){
  float xShift = 0;
  float yShift = 0;
  
  if(!alreadyTranslated){
    glPushMatrix();
    
    xShift = xCoordinate - *_x;
    yShift = yCoordinate - *_y;
  
    glTranslatef(xShift , yShift , 0.0);
  }
  
  int sectorX = (int)xCoordinate / SECTOR_SIZE;
  if(sectorX == NUM_SECTORS_PER_SIDE) sectorX = NUM_SECTORS_PER_SIDE - 1;
  int sectorY = (int)yCoordinate / SECTOR_SIZE;
  if(sectorY == NUM_SECTORS_PER_SIDE) sectorY = NUM_SECTORS_PER_SIDE - 1;
  
  char buf[30];
  
  glEnable(GL_BLEND);
  glColor4f(R,G,B,0.85);
  
  sprintf(buf,"x:  %.2f",xCoordinate);
  printStringToRight(FONT_SMALL,false,&buf[0],xShift,yShift+65);
  sprintf(buf,"y:  %.2f",yCoordinate);
  printStringToRight(FONT_SMALL,false,&buf[0],xShift,yShift+45);
  sprintf(buf,"dx: %.2f",xChange/DT);
  printStringToRight(FONT_SMALL,false,&buf[0],xShift,yShift+25);
  sprintf(buf,"dy: %.2f",yChange/DT);
  printStringToRight(FONT_SMALL,false,&buf[0],xShift,yShift+5);
  sprintf(buf,"ax: %.2f",_sectors[sectorX][sectorY].xAccel/DT/DT);
  printStringToRight(FONT_SMALL,false,&buf[0],xShift,yShift-15);
  sprintf(buf,"ay: %.2f",_sectors[sectorX][sectorY].yAccel/DT/DT);
  printStringToRight(FONT_SMALL,false,&buf[0],xShift,yShift-35);
  sprintf(buf,"m:  %.1f",mass);
  printStringToRight(FONT_SMALL,false,&buf[0],xShift,yShift-55);
  sprintf(buf,"r:  %.0f",radius);
  printStringToRight(FONT_SMALL,false,&buf[0],xShift,yShift-75);
    
  glDisable(GL_BLEND);
  
  if(!alreadyTranslated){
    glPopMatrix();
  }
}



// a debugging tool
void objecttype::drawHeading(bool alreadyTranslated){
  float xShift = 0;
  float yShift = 0;
  
  if(!alreadyTranslated){
    glPushMatrix();
    
    xShift = xCoordinate - *_x;
    yShift = yCoordinate - *_y;
  
    glTranslatef(xShift , yShift , 0.0);
  }
  
  glColor3f(0.0,0.0,1.0);
  glBegin(GL_LINES);
    glVertex2f(50.0,0.0);
    glVertex2f(40.0,10.0);
    glVertex2f(50.0,0.0);
    glVertex2f(40.0,-10.0);
    glVertex2f(50.0,0.0);
    glVertex2f(0.0,0.0);
  glEnd();
  
  glPushMatrix();
  glRotatef(45.0, 0.0 , 0.0 , 1.0);
  
  glColor3f(0.0,1.0,1.0);
  glBegin(GL_LINES);
    glVertex2f(50.0,0.0);
    glVertex2f(40.0,10.0);
    glVertex2f(50.0,0.0);
    glVertex2f(40.0,-10.0);
    glVertex2f(50.0,0.0);
    glVertex2f(0.0,0.0);
  glEnd();
  
  glPopMatrix();
  
  if(!alreadyTranslated){
    glPopMatrix();
  }
}



// map wrap-around function
// keeps things smooth if by some wierdness an object jumps a wall
void objecttype::setInBounds(){
  if(xCoordinate >= MAX_COORDINATE) inBounds = false;
  else if(xCoordinate < 0) inBounds = false;
  else if(yCoordinate >= MAX_COORDINATE) inBounds = false;
  else if(yCoordinate < 0) inBounds = false;
  else inBounds = true;
}



// handles placement of this object into a sector, and also calls fcns ...
// ... to handle any resulting collisions
void objecttype::placeInSector(){
  
  // look for local collision (and deal with any that are found)
  checkSector(&_sectors[xSectorIndex][ySectorIndex]);
  
  // calculate what the coordinates of the neighboring sectors are
  int lessX = xSectorIndex - 1;
  if(lessX < 0) lessX = NUM_SECTORS_PER_SIDE - 1;
  int moreX = xSectorIndex + 1;
  if(moreX >= NUM_SECTORS_PER_SIDE) moreX = 0;
  int lessY = ySectorIndex - 1;
  if(lessY < 0) lessY = NUM_SECTORS_PER_SIDE - 1;
  int moreY = ySectorIndex + 1;
  if(moreY >= NUM_SECTORS_PER_SIDE) moreY = 0;
  
  // check all surrounding sectors, deal with any collisions that are found
  checkSector(&_sectors[lessX][lessY]);
  checkSector(&_sectors[lessX][ySectorIndex]);
  checkSector(&_sectors[lessX][moreY]);
  checkSector(&_sectors[moreX][lessY]);
  checkSector(&_sectors[moreX][ySectorIndex]);
  checkSector(&_sectors[moreX][moreY]);
  checkSector(&_sectors[xSectorIndex][lessY]);
  checkSector(&_sectors[xSectorIndex][moreY]);
  
  if(!isDead){
    // only we we're not dead!
    // this gets us into a sector, so we can be collided with
    addToSector(&_sectors[xSectorIndex][ySectorIndex]);
  }
}



void objecttype::checkSectorRecur(objecttype *tree){
  bool less = tree->xCoordinate - xCoordinate < 0;
  
  if( tree->radius + radius > fabsf(tree->xCoordinate - xCoordinate) ){
    if( tree->radius + radius > fabsf(tree->yCoordinate - yCoordinate) )
      if( tree->radius + radius > distanceFrom(tree) )
        collision(this,tree);
    
    if( tree->sectorLess )
      checkSectorRecur(tree->sectorLess);
    if( tree->sectorMore )
      checkSectorRecur(tree->sectorMore);
  }else{
    if( less && tree->sectorLess )
      checkSectorRecur(tree->sectorLess);
    else if( tree->sectorMore )
      checkSectorRecur(tree->sectorMore);
  }
}



// this function figures out if there really is a collision, and then ...
// ... it calls the function "collision" which is not part of any class ...
// ... which then calls back here with "collisionEffect" to apply ...
// ... this object's fair damage
void objecttype::checkSector(sectortype *sPtr){
  
  // are we dead?  Also, our signal that nothing up to date is in this sector...
  if(isDead || sPtr->timestamp != _timestamp) return;
  
  if( sPtr->first ) checkSectorRecur(sPtr->first);
}



// objects bounce of the edges of the map
void objecttype::collideWithEdges(){
  if(xCoordinate + radius >= MAX_COORDINATE && xChange > 0)
    xChange = -xChange;
  
  if(xCoordinate - radius <= 0 && xChange < 0)
    xChange = -xChange;
  
  if(yCoordinate + radius >= MAX_COORDINATE && yChange > 0)
    yChange = -yChange;
    
  if(yCoordinate - radius <= 0 && yChange < 0)
    yChange = -yChange;
}



void objecttype::gravitate(){
  #ifdef DEBUG_OBJECTTYPE
  if(this == _starCore){
    fprintf(stderr,"WARNG - _starcore is at objecttype::gravitate.\n");
    return;
  }
  #endif
  
  if( !_sectors[xSectorIndex][ySectorIndex].nearCenter ){
    xChange += _sectors[xSectorIndex][ySectorIndex].xAccel;
    yChange += _sectors[xSectorIndex][ySectorIndex].yAccel;
  }else{
    double xDist = MAX_COORDINATE/2 - xCoordinate;
    double yDist = MAX_COORDINATE/2 - yCoordinate;
    double dist = sqrt( xDist * xDist + yDist * yDist );
    double angle = atan( yDist / xDist );
    double accel = DT * DT * GRAVITY_CONST / (dist * dist);
      
    // divide acceleration into x and y components
    if(xDist < 0){
      xChange -= accel * cos(angle);
      yChange -= accel * sin(angle);
    }else{
      xChange += accel * cos(angle);
      yChange += accel * sin(angle);
    }
  }
}



void objecttype::thrust(double faceAngle, double maxSpeed, double enginePower){
  double oldSpeed = sqrt(xChange * xChange + yChange * yChange);
  
  // convert to radians
  faceAngle *= M_PI / 180;
  
  // to fix a special case
  if(xChange == 0) xChange = .0001;
  
  // the angle (direction) we are moving in
  double moveAngle = atan( yChange / xChange );
  if(xChange < 0) moveAngle += M_PI;
  
  // the angle between movement and facing
  double diffAngle = faceAngle - moveAngle;
  if(diffAngle < 0) diffAngle += 2 * M_PI;
  else if(diffAngle > 2 * M_PI) diffAngle -= 2 * M_PI;
  
  // calculate the amount of *raw* thrust 90 degrees different from our movement
  double sidewaysThrust = sin(diffAngle) * enginePower;
  
  // calculate the amound of *raw* thrust in the direction of our heading
  double aheadThrust = cos(diffAngle) * enginePower;
  
  // scale thrust sideways to our movement
  if(sidewaysThrust > 0){
    double scale = 1 - sidewaysThrust / maxSpeed;
    
    if(scale < 0){
      sidewaysThrust = 0;
    }else{
      sidewaysThrust *= scale;
    }
  }else{
    double scale = 1 + sidewaysThrust / maxSpeed;
    
    if(scale < 0){
      sidewaysThrust = 0;
    }else{
      sidewaysThrust *= scale;
    }
  }
  
  // scale thrust in the direction of our heading
  // but only if this thrust is forward (as opposed to stopping thrust)
  // this does leave a "hole" where an ultra-super-powerful engine could do bad things, not a real concern though
  if(aheadThrust > 0){
    double scale = 1 - (oldSpeed + aheadThrust) / maxSpeed;
    
    if(scale < 0){
      aheadThrust = 0;
    }else{
      aheadThrust *= scale;
    }
  }
  
  // apply scaled sideways thrust
  xChange += sidewaysThrust * cos(moveAngle + M_PI / 2);
  yChange += sidewaysThrust * sin(moveAngle + M_PI / 2);
  
  // apply scaled ahead thrust
  xChange += aheadThrust * cos(moveAngle);
  yChange += aheadThrust * sin(moveAngle);
  
  #ifdef DEBUG_THRUST
  // thrust status output (mainly used by the ships at this time)
  thrustStat1 = aheadThrust;
  thrustStat2 = sidewaysThrust;
  thrustStat3 = sqrt(xChange * xChange + yChange * yChange);
  #endif
}
