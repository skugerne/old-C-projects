#include "main.hpp"



void alientype::drawShield(){
  if(shieldGlow <= 0) return;
  
  int sides = (int)radius;
  float aChangePer = M_PI * 2 / radius;
  
  glColor4f(0.0 , 0.0 , 0.0 , 0.0);
  
  glEnable(GL_BLEND);
  glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0 , 0.0);
      
    // select proper color for the shields to be
    glColor4f(0.0 , shieldGlow , 0.0 , 0.6 * shieldGlow);
      
    for(int i=0; i<sides+1; ++i){
      glVertex2f(
        radius * cos((float)i * aChangePer), 
        radius * sin((float)i * aChangePer)
      );
    }
  glEnd();
  glDisable(GL_BLEND);
}



// places entry at proper (distance sorted) place in array
// NOTE: not intended for huge numbers of targets, slow for that
inline void insert(targettype *targs, int numTargs, float bearing,
  float distance, float heading, float approach, float speed
){
  if( distance > targs[numTargs-1].distance ) return;
  
  int i;
  for(i=numTargs-2;i>=0;i--){
    if( targs[i].distance > distance ){
      //printf("Shift %d down (%.2f > %.2f).\n",i,targs[i].distance,distance);
      targs[i+1].bearing = targs[i].bearing;
      targs[i+1].distance = targs[i].distance;
      targs[i+1].heading = targs[i].heading;
      targs[i+1].approach = targs[i].approach;
      targs[i+1].speed = targs[i].speed;
    }else
      break;
  }
  
  //printf("Setting index %d.\n",i+1);
  targs[i+1].bearing = bearing / M_PI * 180;
  targs[i+1].distance = distance;
  targs[i+1].heading = heading / M_PI * 180;
  targs[i+1].approach = approach / M_PI * 180;
  targs[i+1].speed = speed;
}



// identify targets within a given range
// place into targettype array, return number of targets found
int alientype::findTargets(targettype *targs, int numTargs, float maxRange){
  // just call the other fcn with huge angle ranges
  return findTargetsAngleApp(targs,numTargs,maxRange,-360,360,360);
}



// identify targets within a given range and angle of approach
// place into targettype array, return number of targets found
int alientype::findTargetsApp(targettype *targs, int numTargs, float maxRange,
  float maxApproach
){
  // just call the other fcn with huge angle ranges
  return findTargetsAngleApp(targs,numTargs,maxRange,-360,360,maxApproach);
}



// identify targets within a given range and angle range
// place into targettype array, return number of targets found
int alientype::findTargetsAngle(targettype *targs, int numTargs, float maxRange,
  float minAngle, float maxAngle
){
  // just call the other fcn with huge angle ranges
  return findTargetsAngleApp(targs,numTargs,maxRange,minAngle,maxAngle,360);
}



// identify targets within a given range, angle range and approach angle range
// place into targettype array, return number of targets found
int alientype::findTargetsAngleApp(targettype *targs, int numTargs,
  float maxRange, float minAngle, float maxAngle, float maxApproach
){
  // note that this is blatantly not the right way to aquire targets
  
  int targetsFound = 0;
  
  // scale these to radians
  maxAngle = maxAngle / 180 * M_PI;
  minAngle = minAngle / 180 * M_PI;
  maxApproach = maxApproach / 180 * M_PI;
  
  int sectorX = (int)xCoordinate / SECTOR_SIZE;
  int sectorY = (int)yCoordinate / SECTOR_SIZE;
  int secOffset = 1 + (int)maxRange / SECTOR_SIZE;
  
  // set all ranges to "very far"
  for(int i=0;i<numTargs;++i)
    targs[i].distance = NO_TARGET;
  
  for(int i=sectorX-secOffset;i<sectorX+secOffset;++i){
    if(i >= 0 && i < NUM_SECTORS_PER_SIDE){
      for(int j=sectorY-secOffset;j<sectorY+secOffset;++j){
        if(j >= 0 && j < NUM_SECTORS_PER_SIDE){
          if(_sectors[i][j].timestamp >= _timestamp-1){
            // should ignore sections holding stale data
            
            objecttype *oPtr = _sectors[i][j].first;
            while(oPtr){
              
              // do not look at yourself
              if( oPtr == this ){
                oPtr = oPtr->getSecNext();
                continue;
              }
              
              // find the distance of the object
              float distX = xCoordinate - oPtr->x();
              float distY = yCoordinate - oPtr->y();
              float dist = sqrt(distX * distX + distY * distY);
              
              // cull things which are out of range or not close enough to count
              if( dist > maxRange && dist < targs[numTargs-1].distance ){
                oPtr = oPtr->getSecNext();
                continue;
              }
              
              // find the bearing of the object
              float bearing = atan( distY / distX );
              if(distX > 0) bearing += M_PI;
              
              // cull things which are out of our angle range
              if( bearing < minAngle || bearing > maxAngle ){
                oPtr = oPtr->getSecNext();
                continue;
              }
              
              // find the relative heading of the object
              float movX = xChange - oPtr->dx();
              float movY = yChange - oPtr->dy();
              float heading = atan( movY / movX );
              if(movX > 0) heading += M_PI;
              
              float approach = M_PI + bearing - heading;
              while(approach > M_PI) approach -= 2*M_PI;
              
              // cull things which are not heading for the alien
              if( fabsf(approach) > maxApproach ){
                oPtr = oPtr->getSecNext();
                continue;
              }
              
              float speed = sqrt(movX * movX + movY * movY);
              
              insert(targs,numTargs,bearing,dist,heading,approach,speed);
              ++targetsFound;
              
              oPtr = oPtr->getSecNext();
            }
          }  // end if
        }  // end if
      }
    }  // end if
  }
  
  return targetsFound < numTargs ? targetsFound : numTargs;
}

