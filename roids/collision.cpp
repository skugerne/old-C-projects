#include "main.hpp"



// called through main update functions, through objects, while they are ...
// ... placing themselves into the sectors "structure"
void collision(objecttype *obj1, objecttype *obj2){

  #ifdef DEBUG_COLLISIONS
  if(obj1 == NULL || obj2 == NULL){
    fprintf(stderr,"ERROR - NULL input at collision.\n");
    return;
  }
  
  if(obj1 == obj2){
    fprintf(stderr,"ERROR - objects are identical at collision().\n");
    return;
  }
  
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"COLLISION: < obj1=%u, obj2=%u >\n",obj1->getID(),obj2->getID());
  #endif
  #endif
  
  double dx, dy, x, y;
  double dxy, v, v1, v2, nullV;
  double incomingAngle, centersAngle, nullAngle;
  double mass1 = obj1->m();
  double mass2 = obj2->m();
  
  #ifdef DEBUG_COLLISIONS
  // calculate the starting energies (for results-checking only)
  double t1, t2;
  t1 = fabsf(obj1->dx());
  t2 = fabsf(obj1->dy());
  double inert1 = (t1*t1 + t2*t2) * mass1;
  t1 = fabsf(obj2->dx());
  t2 = fabsf(obj2->dy());
  double inert2 = (t1*t1 + t2*t2) * mass2;
  #endif
  
  // make things stationary around the 2nd object to ease calculations
  dx = obj1->dx() - obj2->dx();
  dy = obj1->dy() - obj2->dy();
  
  // make 2nd object's coordinates 0,0 to ease calculations
  x = obj1->x() - obj2->x();
  y = obj1->y() - obj2->y();
  
  // not moving relative to each other, therefore cannot be colliding
  if(dx == 0 && dy == 0 ){
    #ifdef DEBUG_COLLISIONS
    fprintf(stderr,"Potential collision dropped, objects at same speeds.\n");
    #endif
    
    return;
  }
  
  // forget all the special cases
  if(x == 0) x = .0001;
  if(y == 0) y = .0001;
  if(dx == 0) dx = .0000001;     // scaled for a DT of 1/1000
  if(dy == 0) dy = .0000001;     // scaled for a DT of 1/1000
  
  // speed of closure
  dxy = sqrt(dx * dx + dy * dy);      // closing velocity
  
  // angle that the 1st object is "approaching" from
  incomingAngle = atan(dy/dx);
  if(dx < 0) incomingAngle += M_PI;
  if(incomingAngle < 0) incomingAngle += 2 * M_PI;
  // should result in a 0-360 deg range of angles
  // 270 to 360: lower right
  // 180 to 270: lower left
  // 90 to 180:  upper left
  // 0 to 90:    upper right
  
  // angle that the object centers are at
  centersAngle = atan(y/x);
  if(x < 0) centersAngle += M_PI;
  if(centersAngle < 0) centersAngle += 2 * M_PI;
  // should result in a 0-360 deg range of angles
  // 270 to 360: lower right
  // 180 to 270: lower left
  // 90 to 180:  upper left
  // 0 to 90:    upper right
  
  // if the angles are within 90 degrees, there can be no colission
  // 180 degree separation means straight-on collision
  if( fabsf(incomingAngle - centersAngle) < M_PI / 2 ) return;
  if( fabsf(incomingAngle - centersAngle) > 3 * M_PI / 2 ) return;
    
  nullAngle = incomingAngle - centersAngle;
  nullV = dxy * sin(nullAngle);    // non-collision v
  v = dxy * cos(nullAngle);        // collision v, head-on
    
  v1 = v * ((mass1 - mass2) / (mass2 + mass1));
  v2 = v * ((2 * mass1) / (mass1 + mass2));

  // the actual application of the accelerations
  obj1->xChange = 
    obj2->dx() - (sin(centersAngle) * nullV) + (cos(centersAngle) * v1);
  obj1->yChange = 
    obj2->dy() + (cos(centersAngle) * nullV) + (sin(centersAngle) * v1);
  obj2->xChange = obj2->dx() + (cos(centersAngle) * v2);
  obj2->yChange = obj2->dy() + (sin(centersAngle) * v2);
  
  // setting up objects for retransmit (no real effect on client)
  objectcategorytype type1 = obj1->getCollisionMod();
  objectcategorytype type2 = obj2->getCollisionMod();
  
  double warhead1 = obj1->getWarhead(type2);
  double warhead2 = obj2->getWarhead(type1);
  
  // damage calculation ... hopefully that is "right"
  if(fabsf(v) < .001){                      // scaled for a DT of 1/1000 ... 1 PPS
    obj1->collisionEffect( 0, type2 );
    obj2->collisionEffect( 0, type1 );
  }else{
    obj1->collisionEffect( v*v*mass2 + warhead2, type2 );
    obj2->collisionEffect( v*v*mass1 + warhead1, type1 );
  }
  
  #ifdef DEBUG_COLLISIONS
  // finding the new energies
  t1 = fabsf(obj1->dx());
  t2 = fabsf(obj1->dy());
  double inert1_b = (t1*t1 + t2*t2) * mass1;
  t1 = fabsf(obj2->dx());
  t2 = fabsf(obj2->dy());
  double inert2_b = (t1*t1 + t2*t2) * mass2;
  
  // making sure that the collisions conserve energy
  if(( (inert1_b + inert2_b) * 1.001 ) < (inert1 + inert2) ||
    ( (inert1_b + inert2_b) * 0.999 ) > (inert1 + inert2)
  ){
    fprintf(stderr,"WARNG - bad collision:\n");
    fprintf(stderr,"%f != %f.\n",inert1_b + inert2_b,inert1 + inert2);
    //exit(1);
  }else{
    fprintf(stderr,"Collision concluded normally.\n");
  }
  #endif
}



// particle collisions with objects, note that particles always call this and ...
// ... not the objects, because particles update separately
void collision(
  float px, float py, float *pdx, float *pdy, objecttype *oPtr, objectcategorytype what
){
  
  #ifdef DEBUG_COLLISIONS
  if(oPtr == NULL){
    fprintf(stderr,"ERROR - NULL input at collision.\n");
    return;
  }
  #endif
  
  double dx, dy, x, y;
  double dxy, v, v1, v2, nullV;
  double incomingAngle, centersAngle, nullAngle;
  
  //fprintf(stderr,"(%0.3f,%0.3f) d(%0.3f,%0.3f) (%0.3f,%0.3f) d(%0.3f,%0.3f)\n",
  //  px,py,*pdx,*pdy,oPtr->x(),oPtr->y(),oPtr->dx(),oPtr->dy()
  //);
  
  // make things stationary around the object to ease calculations
  dx = *pdx - oPtr->dx();
  dy = *pdy - oPtr->dy();
  
  // make object's coordinates 0,0 to ease calculations
  x = px - oPtr->x();
  y = py - oPtr->y();
  
  // not moving relative to each other, therefore cannot be colliding
  if(dx == 0 && dy == 0){
    #ifdef DEBUG_COLLISIONS
    fprintf(stderr,"Potential particle-object collision dropped, at same speeds.\n");
    #endif
    
    return;
  }
  
  // forget all the special cases
  if(x == 0) x = .0001;
  if(y == 0) y = .0001;
  if(dx == 0) dx = .0000001;     // scaled for a DT of 1/1000
  if(dy == 0) dy = .0000001;     // scaled for a DT of 1/1000
  
  // speed of closure
  dxy = sqrt(dx * dx + dy * dy);      // closing velocity
  
  //fprintf(stderr,"Object: %d.\n",oPtr->getCollisionMod());
  //fprintf(stderr,"(%0.3f,%0.3f) d(%0.3f,%0.3f) dxy(%0.3f) ",x,y,dx,dy,dxy);
  
  // angle that the particle is "approaching" from
  incomingAngle = atan(dy/dx);
  if(dx < 0) incomingAngle += M_PI;
  if(incomingAngle < 0) incomingAngle += 2 * M_PI;
  // should result in a 0-360 deg range of angles
  // 270 to 360: lower right
  // 180 to 270: lower left
  // 90 to 180:  upper left
  // 0 to 90:    upper right
  
  // angle that the particle and object centers are at
  centersAngle = atan(y/x);
  if(x < 0) centersAngle += M_PI;
  if(centersAngle < 0) centersAngle += 2 * M_PI;
  // should result in a 0-360 deg range of angles
  // 270 to 360: lower right
  // 180 to 270: lower left
  // 90 to 180:  upper left
  // 0 to 90:    upper right
  
  //fprintf(stderr,"angles(%0.3f,%0.3f)\n",incomingAngle,centersAngle);
  
  // if the angles are within 90 degrees, there can be no colission
  // 180 degree separation means straight-on collision
  if( fabsf(incomingAngle - centersAngle) < M_PI / 2 ) return;
  if( fabsf(incomingAngle - centersAngle) > 3 * M_PI / 2 ) return;
    
  nullAngle = incomingAngle - centersAngle;
  nullV = dxy * sin(nullAngle);    // non-collision v
  v = dxy * cos(nullAngle);        // collision v, head-on
    
  v1 = v * -1;
  v2 = 0;

  // the actual application of the forces (only to the particle, note)
  *pdx = 
    oPtr->dx() - (sin(centersAngle) * nullV) + (cos(centersAngle) * v1);
  *pdy = 
    oPtr->dy() + (cos(centersAngle) * nullV) + (sin(centersAngle) * v1);
  
  oPtr->collisionEffect( 0, what );
        
  #ifdef DEBUG_COLLISIONS
  fprintf(stderr,"Particle-object collision resolved normally.\n");
  #endif
}

