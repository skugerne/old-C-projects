#include "main.hpp"



// for over-the-network creation
startype::startype(double X, double Y, double DX, double DY, double M){
  xCoordinate = X;
  yCoordinate = Y;
  xChange = DX;
  yChange = DY;
  mass = M;
  hasGravity = true;
  
  // variable used for the usual object interactions, not for drawing
  radius = STAR_RADIUS;
  
  collisionModifier = COLLIDE_STAR;
  
  basicInit();
}



void startype::draw(){
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
  
  glPushMatrix();
  glTranslatef(xShift , yShift , 0.0);
  
  glEnable(GL_BLEND);
  
  glColor4f(1,1,.5,.5);
  glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    for(int i=0;i<STAR_SIDES;i+=2){
      glColor4f(1,1,0,0);
      glVertex2f(
        STAR_RAYS * cos( 2 * M_PI * ((double)i / STAR_SIDES) ),
        STAR_RAYS * sin( 2 * M_PI * ((double)i / STAR_SIDES) )
      );
      glColor4f(1,.9,.3,.1);
      glVertex2f(
        STAR_RAYS * cos( 2 * M_PI * ((double)(i+1) / STAR_SIDES) ),
        STAR_RAYS * sin( 2 * M_PI * ((double)(i+1) / STAR_SIDES) )
      );
    }
    glColor4f(1,1,0,0);
    glVertex2f(STAR_RAYS,0);
  glEnd();
  
  glBegin(GL_TRIANGLES);
    for(int i=0;i<STAR_SIDES;i+=2){
      glColor4f(1,1,0,0);
      glVertex2f(
        STAR_RAYS * cos( 2 * M_PI * ((double)i / STAR_SIDES) ),
        STAR_RAYS * sin( 2 * M_PI * ((double)i / STAR_SIDES) )
      );
      glColor4f(1,.9,.3,.1);
      glVertex2f(
        STAR_RAYS * cos( 2 * M_PI * ((double)(i+1) / STAR_SIDES) ),
        STAR_RAYS * sin( 2 * M_PI * ((double)(i+1) / STAR_SIDES) )
      );
      glColor4f(1,.9,.3,0);
      glVertex2f(
        2 * STAR_RAYS * cos( 2 * M_PI * (((double)i+.5) / STAR_SIDES) ),
        2 * STAR_RAYS * sin( 2 * M_PI * (((double)i+.5) / STAR_SIDES) )
      );
      glVertex2f(
        2 * STAR_RAYS * cos( 2 * M_PI * (((double)i+.5) / STAR_SIDES) ),
        2 * STAR_RAYS * sin( 2 * M_PI * (((double)i+.5) / STAR_SIDES) )
      );
      glColor4f(1,1,0,0);
      glVertex2f(
        STAR_RAYS * cos( 2 * M_PI * ((double)(i+2) / STAR_SIDES) ),
        STAR_RAYS * sin( 2 * M_PI * ((double)(i+2) / STAR_SIDES) )
      );
      glColor4f(1,.9,.3,.1);
      glVertex2f(
        STAR_RAYS * cos( 2 * M_PI * ((double)(i+1) / STAR_SIDES) ),
        STAR_RAYS * sin( 2 * M_PI * ((double)(i+1) / STAR_SIDES) )
      );
    }
  glEnd();
  
  glDisable(GL_BLEND);
  
  glColor3f(1,1,1);
  glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    glColor3f(1,1,0);
    for(int i=0;i<STAR_SIDES;++i){
      glVertex2f(
        (double)STAR_RADIUS * cos( 2 * M_PI * ((double)i / STAR_SIDES) ),
        (double)STAR_RADIUS * sin( 2 * M_PI * ((double)i / STAR_SIDES) )
      );
    }
    glVertex2f(STAR_RADIUS,0);
  glEnd();
  
  glPopMatrix();
}



// moves it around
objecttype* startype::specialUpdate(){

  xCoordinate = MAX_COORDINATE / 2;
  yCoordinate = MAX_COORDINATE / 2;
  
  xChange = 0;
  yChange = 0;
  
  // so things can collide with it
  placeInSector();
  
  // doesn't really matter... not usually included in the _objects list
  return next;
}
