#include "main.hpp"



// convert strings to shotnametype
shotnametype shotnametypeFromString(const char *in){
  if(strcmp(in,"SHOT_WEAK") == 0) return SHOT_WEAK;
  if(strcmp(in,"SHOT_MED") == 0) return SHOT_MED;
  if(strcmp(in,"SHOT_BIG") == 0) return SHOT_BIG;
  if(strcmp(in,"SHOT_SUPER") == 0) return SHOT_SUPER;
  fprintf(stderr,"Failed to translate %s.\n",in);
  exit(1);
}

// convert strings to shotcounttype
shotcounttype shotcounttypeFromString(const char *in){
  if(strcmp(in,"SHOT_SINGLE") == 0) return SHOT_SINGLE;
  if(strcmp(in,"SHOT_DOUBLE") == 0) return SHOT_DOUBLE;
  if(strcmp(in,"SHOT_TRIPPLE") == 0) return SHOT_TRIPPLE;
  if(strcmp(in,"SHOT_QUAD") == 0) return SHOT_QUAD;
  fprintf(stderr,"Failed to translate %s.\n",in);
  exit(1);
}

// convert strings to shotmodtype
shotmodtype shotmodtypeFromString(const char *in){
  if(strcmp(in,"SHOT_NORM") == 0) return SHOT_NORM;
  if(strcmp(in,"SHOT_FAST") == 0) return SHOT_FAST;
  if(strcmp(in,"SHOT_VERY_FAST") == 0) return SHOT_VERY_FAST;
  if(strcmp(in,"SHOT_HEAVY") == 0) return SHOT_HEAVY;
  if(strcmp(in,"SHOT_FAST_HEAVY") == 0) return SHOT_FAST_HEAVY;
  if(strcmp(in,"SHOT_PROX") == 0) return SHOT_PROX;
  if(strcmp(in,"SHOT_LONG") == 0) return SHOT_LONG;
  if(strcmp(in,"SHOT_FAST_LONG") == 0) return SHOT_FAST_LONG;
  fprintf(stderr,"Failed to translate %s.\n",in);
  exit(1);
}



// for over-the-network creation
shottype::shottype(objecttype *launcher, double lAngle,
  shotnametype name, shotcounttype count, shotmodtype mod
){
  if(!launcher){
    fprintf(stderr,"ERROR - shottype passed NULL launcher.\n");
    isDead = true;
    return;
  }

  // recursively handle multiple shots
  switch (count){
    case SHOT_DOUBLE:{
      objecttype *oPtr;
      oPtr = new shottype(launcher,lAngle+5,name,SHOT_SINGLE,mod);
      oPtr->addToNewList();
      lAngle -= 5;
      break;
    }case SHOT_TRIPPLE:{
      objecttype *oPtr;
      oPtr = new shottype(launcher,lAngle,name,SHOT_SINGLE,mod);
      oPtr->addToNewList();
      oPtr = new shottype(launcher,lAngle+10,name,SHOT_SINGLE,mod);
      oPtr->addToNewList();
      lAngle -= 10;
      break;
    }case SHOT_QUAD:{
      objecttype *oPtr;
      oPtr = new shottype(launcher,lAngle-5,name,SHOT_SINGLE,mod);
      oPtr->addToNewList();
      oPtr = new shottype(launcher,lAngle+5,name,SHOT_SINGLE,mod);
      oPtr->addToNewList();
      oPtr = new shottype(launcher,lAngle+15,name,SHOT_SINGLE,mod);
      oPtr->addToNewList();
      lAngle -= 15;
      break;
    }default:
      break;
  }
  
  shotName = name;
  shotMod = mod;
  
  double speedFactor;
  
  switch (mod) {
    case SHOT_VERY_FAST:
      speedFactor = 1.75;
      break;
    case SHOT_FAST:
    case SHOT_FAST_HEAVY:
    case SHOT_FAST_LONG:
      speedFactor = 1;
      break;
    default:
      speedFactor = .5;
      break;
  }
  
  // lAngle is expected in units of degrees
  lAngle *= M_PI / 180;
  
  xCoordinate = launcher->x() + launcher->r() * cos(lAngle);
  yCoordinate = launcher->y() + launcher->r() * sin(lAngle);
  xChange = launcher->dx() + speedFactor * cos(lAngle);
  yChange = launcher->dy() + speedFactor * sin(lAngle);
  
  switch (name) {
    case SHOT_WEAK:
      mass = .5;
      radius = 4;
      warhead = 1;
      break;
    case SHOT_MED:
      mass = 1;
      radius = 5;
      warhead = 3;
      break;
    case SHOT_BIG:
      mass = 1.5;
      radius = 6;
      warhead = 9;
      break;
    case SHOT_SUPER:
      mass = 2.5;
      radius = 7;
      warhead = 27;
      break;
  }
  
  switch (mod) {
    case SHOT_LONG:
    case SHOT_FAST_LONG:
      mSecLeft = 5000;
      break;
    case SHOT_PROX:
      radius *= 4;
      mSecLeft = 1000;
      break;
    default:
      mSecLeft = 1000;
      break;
  }
  
  basicInit();
  
  collisionModifier = COLLIDE_WEAPON;
  
  baseVisibility = 1;
  detectabilityFactor = 1 * radius;
  
  //fprintf(stderr,"Shot %d made.\n",idNum);
}



// moves it around
objecttype* shottype::specialUpdate(){

  // timer...
  --mSecLeft;
  if(mSecLeft < 0) explode();
  
  return next;
}



void shottype::draw(){
  glPushMatrix();
  
  float xShift = xCoordinate - *_x;
  float yShift = yCoordinate - *_y;
  
  glTranslatef(xShift , yShift , 0);
  
  switch (shotMod) {
    case SHOT_NORM:
    case SHOT_FAST:
    case SHOT_VERY_FAST:
    case SHOT_LONG:
    case SHOT_FAST_LONG:
      glScalef(2,2,2);
      glBegin(GL_TRIANGLE_FAN);
        glColor3f(1,1,1);
        glVertex2f(0,0);
        glColor3f(.6,.6,.6);
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
      break;
    case SHOT_HEAVY:
    case SHOT_FAST_HEAVY:
      glScalef(3,3,3);
      glBegin(GL_TRIANGLE_FAN);
        glColor3f(1,1,1);
        glVertex2f(0,0);
        glColor3f(.6,.6,0);
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
      break;
    case SHOT_PROX:
      glScalef(4,4,4);
      glBegin(GL_TRIANGLE_FAN);
        if(_time / 300 % 2 == 0){
          glColor3f(1,1,0);
        }else{
          glColor3f(0,0,0);
        }
        glVertex2f(0,0);
        glColor3f(.6,.6,0);
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
      break;
    #ifdef DEBUG
    default:
      fprintf(stderr,"ERROR - Undefined value for shotMod at shottype::draw.\n");
    #endif
  }
  
  glPopMatrix();
  
  if(_option1){
    drawRadius(false);
  }
}



void shottype::collisionEffect(double damage, objectcollisiontype what){
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"Shot %d collide.\n",idNum);
  #endif
  
  if(damage > 0 && what != COLLIDE_DEFLECT)
    explode();
}



double shottype::getWarhead(objectcollisiontype other){
  if(other == COLLIDE_DEFLECT && shotMod != SHOT_PROX) return 0.0;
  else return warhead;
}



void shottype::explode(){
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"Shot %d explode.\n",idNum);
  #endif
  
  for(int i=0;i<10;++i){
    double speedFactor = 
      (double)rand() / ( (double)RAND_MAX+1.0 );
      
    double tempA =
      2 * M_PI * ( (double)rand() / ( (double)RAND_MAX+1.0 ) );
    
    double particleDX, particleDY;
    
    particleDX = xChange - speedFactor * .3 * cos(tempA);
    particleDY = yChange - speedFactor * .3 * sin(tempA);
    createFlame(xCoordinate,yCoordinate,particleDX,particleDY);
  }
  
  for(int i=0;i<5;++i)
    createDust(xCoordinate,yCoordinate,xChange,yChange,1);
  
  isDead = true;
}
