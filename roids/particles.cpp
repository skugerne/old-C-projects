#include "main.hpp"



bool checkCollideNow;



void initParticles(){
  _flame.particles = new particletype[NUM_FLAME];
  _flame.validBegin = 0;
  _flame.validEnd = 0;
  _flame.count = NUM_FLAME;
  _flame.what = COLLIDE_FLAME;
  
  _dust.particles = new particletype[NUM_DUST];
  _dust.validBegin = 0;
  _dust.validEnd = 0;
  _dust.count = NUM_DUST;
  _dust.what = COLLIDE_DUST;
}



// neatens up collision detection some
void particleCollideWrapper(
  int sectorX, int sectorY, float x, float y, float *dx, float *dy, objectcollisiontype what
){
  objecttype *nodePtr;
  
  if( _sectors[sectorX][sectorY].timestamp == _timestamp ){
    nodePtr = _sectors[sectorX][sectorY].first;
    
    while( 1 ){
      float distance = nodePtr->r();
      float distX = nodePtr->x() - x;
      float distY = nodePtr->y() - y;
      
      distance *= -distance;
      distance += distX * distX + distY * distY;
      
      if( distance <= 0 )
        collision(x,y,dx,dy,nodePtr,what);
      
      if( nodePtr->x() > x )
        nodePtr = nodePtr->getSecMore();
      else
        nodePtr = nodePtr->getSecLess();
      
      if( nodePtr == NULL ) break;
    }
  }
}



void particleCheckCollisions(
  int sectorX, int sectorY, float x, float y, float *dx, float *dy, objectcollisiontype what
){
  if(checkCollideNow){
    
    // look in the sector that the particle is actually in
    particleCollideWrapper(sectorX,sectorY,x,y,dx,dy,what);
    
    // calculate what the coordinates of the neighboring sectors are
    int lessX = sectorX - 1;
    int moreX = sectorX + 1;
    int lessY = sectorY - 1;
    int moreY = sectorY + 1;
  
    if(lessX >= 0){
      
      // check upper left
      if(lessY >= 0)
        particleCollideWrapper(lessX,lessY,x,y,dx,dy,what);
      
      // check left
      particleCollideWrapper(lessX,sectorY,x,y,dx,dy,what);
      
      // check lower left
      if(moreY < NUM_SECTORS_PER_SIDE)
        particleCollideWrapper(lessX,moreY,x,y,dx,dy,what);
    }
    
    if(moreX < NUM_SECTORS_PER_SIDE){
      
      // check upper right
      if(lessY >= 0)
        particleCollideWrapper(moreX,lessY,x,y,dx,dy,what);
      
      // check right
      particleCollideWrapper(moreX,sectorY,x,y,dx,dy,what);
      
      // check lower right
      if(moreY < NUM_SECTORS_PER_SIDE)
        particleCollideWrapper(moreX,moreY,x,y,dx,dy,what);
    }
    
    // check up
    if(lessY >= 0)
      particleCollideWrapper(sectorX,lessY,x,y,dx,dy,what);
    
    // check down
    if(moreY < NUM_SECTORS_PER_SIDE)
      particleCollideWrapper(sectorX,moreY,x,y,dx,dy,what);
  }
}



void drawFlame(int index){
  if(_flame.particles[index].msLeft == 0) return;
  
  float xShift = _flame.particles[index].x - *_x;
  float yShift = _flame.particles[index].y - *_y;
  
  // only draw if its sorta close to ship
  if(xShift * xShift + yShift * yShift < _virtualPosX * _virtualPosX * 1.5){

    float scale = 4 * (1 - _flame.particles[index].msLeft / 200.0);
    scale *= scale;
    scale += 2;
  
    glPushMatrix();
    glTranslatef(xShift , yShift , 0);
    glScalef(scale, scale, 1.0);
  
    glColor4f( 1.0, 0.7, 0.0, ( _flame.particles[index].msLeft / 200.0 ) );
    glBegin(GL_TRIANGLE_FAN);
      glVertex2f(0,0);
      glColor4f( 1.0, 0.5, 0.0, ( _flame.particles[index].msLeft / 400.0 ) );
      glVertex2f(.707,.707);
      glVertex2f(1,0);
      glVertex2f(.707,-.707);
      glVertex2f(0,-1);
      glVertex2f(-.707,-.707);
      glVertex2f(-1,0);
      glVertex2f(-.707,.707);
      glVertex2f(0,1);
      glVertex2f(.707,.707);
    glEnd();
   
    glPopMatrix();
  
  }
}



void drawFlame(){
  // nothing to do
  if(_flame.validEnd == _flame.validBegin) return;
  
  glEnable(GL_BLEND);
  
  Uint i = _flame.validBegin;
  
  if(i < _flame.validEnd){
    // valid flame particles are >= i and <= NUM_FLAME
    while(i < _flame.validEnd){
      drawFlame(i);
      ++i;
    }
  }else{
    // valid flame particles wrap back around to zero
    while(i < NUM_FLAME){
      drawFlame(i);
      ++i;
    }
    
    // wrap around...
    i = 0;
    while(i < _flame.validEnd){
      drawFlame(i);
      ++i;
    }
  }
  
  glDisable(GL_BLEND);
}



void drawDust(int index){
  if(_dust.particles[index].msLeft <= 0) return;
  
  float xShift = _dust.particles[index].x - *_x;
  float yShift = _dust.particles[index].y - *_y;
  
  // only draw if its sorta close to ship
  if(xShift * xShift + yShift * yShift < _virtualPosX * _virtualPosX * 1.5){

    glColor3f( 0.7, 0.7, 0.7);
    
    glBegin(GL_POINTS);
      glVertex2f(_dust.particles[index].x - *_x , _dust.particles[index].y - *_y);
    glEnd();  
  }
}



void drawDust(){
  // nothing to do
  if(_dust.validEnd == _dust.validBegin) return;
  
  Uint i = _dust.validBegin;
  
  if(i < _dust.validEnd){
    // valid flame particles are >= i and <= NUM_FLAME
    while(i < _dust.validEnd){
      drawDust(i);
      ++i;
    }
  }else{
    // valid flame particles wrap back around to zero
    while(i < NUM_DUST){
      drawDust(i);
      ++i;
    }
    
    // wrap around...
    i = 0;
    while(i < _dust.validEnd){
      drawDust(i);
      ++i;
    }
  }
}



void updateParticle(particlesystemtype *system, Uint index){
  
  particletype *part = &system->particles[index];
  
  // handling timed-out particles
  if(part->msLeft == 0){
    if(index == system->validBegin){
      ++system->validBegin;
      if(system->validBegin >= system->count) system->validBegin = 0;
    }
    return;
  }
  
  // lifetime ... pre-scaled for a DT of 1/1000
  --part->msLeft;
  
  // don't do anything except age if we're far away from the ship
  float xShift = part->x - *_x;
  float yShift = part->y - *_y;
  
  // we should be far enough away to do nothing
  if(xShift * xShift + yShift * yShift > _virtualPosX * _virtualPosX * 4){
    part->msLeft = 0;   // kill it
    return;
  }
  
  // sector calculation
  int sectorX = (int)part->x / SECTOR_SIZE;
  int sectorY = (int)part->y / SECTOR_SIZE;
  
  // make sure we're in bounds
  if(part->x > 0 && sectorX < NUM_SECTORS_PER_SIDE &&
    part->y > 0 && sectorY < NUM_SECTORS_PER_SIDE )
  {
  
    // gravitate ... care only when close to star
    if( _sectors[sectorX][sectorY].nearCenter ){
      part->dx += _sectors[sectorX][sectorY].xAccel;
      part->dy += _sectors[sectorX][sectorY].yAccel;
    }
    
    // handle particle collisions in a semi-generic way
    particleCheckCollisions(
      sectorX,sectorY,
      part->x,part->y,
      &part->dx,&part->dy,
      system->what
    );
    
  }else{
    // time to bounce off that there wall
  
    if(part->x <= 0 && part->dx < 0){
      part->dx *= -1;
    }else if(part->x >= MAX_COORDINATE && part->dx > 0){
      part->dx *= -1;
    }
    
    if(part->y <= 0 && part->dy < 0){
      part->dy *= -1;
    }else if(part->y >= MAX_COORDINATE && part->dy > 0){
      part->dy *= -1;
    }
  }
  
  // drift
  part->x += part->dx;
  part->y += part->dy;
}



void updateParticles(particlesystemtype *system){
  
  // we only do particle collision checking on some updates to speed things up
  if( _timestamp % 16 == 0 )
    checkCollideNow = true;
  else
    checkCollideNow = false;
  
  // nothing to do
  if(system->validEnd == system->validBegin) return;
  
  Uint i = system->validBegin;
  
  if(i < system->validEnd){
    // valid particles are >= i and <= system->count
    while(i < system->validEnd){
      updateParticle(system,i);
      ++i;
    }
  }else{
    // valid particles wrap back around to zero
    while(i < system->count){
      updateParticle(system,i);
      ++i;
    }
    
    // wrap around...
    i = 0;
    while(i < system->validEnd){
      updateParticle(system,i);
      ++i;
    }
  }
}



void updateFlame(){
  #ifdef DEBUG_PARTICLES
  fprintf(stderr,"Updating flame: %d -> %d.\n",_flame.validBegin,_flame.validEnd);
  #endif
  
  updateParticles(&_flame);
}



void updateDust(){
  #ifdef DEBUG_PARTICLES
  fprintf(stderr,"Updating dust: %d -> %d.\n",_dust.validBegin,_dust.validEnd);
  #endif
  
  updateParticles(&_dust);
}

