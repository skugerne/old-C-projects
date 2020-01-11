#include "main.hpp"



#define XX 0
#define YY 1
#define BRIGHT 2



starfieldtype::starfieldtype(int number, float intensity, float in_speed){
  speed = in_speed;
  
  maxGoodCoord = sqrt((float)((_virtualPosX * _virtualPosX) + (_virtualPosY * _virtualPosY)));
  
  maxStars = number;
  if(maxStars > 512) maxStars = 512;
  if(maxStars < 100) maxStars = 100;
  
  for(int i=0;i<maxStars;++i){
    stars[i][XX] = 2.0 * maxGoodCoord * (float)rand() / ((float)RAND_MAX+1.0) - maxGoodCoord;
    stars[i][YY] = 2.0 * maxGoodCoord * (float)rand() / ((float)RAND_MAX+1.0) - maxGoodCoord;
    
    stars[i][BRIGHT] = (float)rand() / ((float)RAND_MAX+1.0) * intensity / 2;
    stars[i][BRIGHT] += intensity / 2;
  }
  
  // initialize after _player has been created because these variables ...
  // ... lock onto variables located in _player
  lastX = *_x;
  lastY = *_y;
}



void starfieldtype::update(){
  xChange = (*_x - lastX) * speed;
  yChange = (*_y - lastY) * speed;
  
  lastX = *_x;
  lastY = *_y;
  
  for(int i=0;i<maxStars;++i){
    stars[i][XX] -= xChange;
    stars[i][YY] -= yChange;
    
    // deal with stars going out of the view
    if(stars[i][XX] > maxGoodCoord){
      stars[i][XX] = (stars[i][XX] - 2 * maxGoodCoord);
    }else if(stars[i][XX] < -maxGoodCoord){
      stars[i][XX] = (stars[i][XX] + 2 * maxGoodCoord);
    }else if(stars[i][YY] > maxGoodCoord){
      stars[i][YY] = (stars[i][YY] - 2 * maxGoodCoord);
    }else if(stars[i][YY] < -maxGoodCoord){
      stars[i][YY] = (stars[i][YY] + 2 * maxGoodCoord);
    }
    
  }
}



void starfieldtype::draw(){
  
  float xlen = *_dx * 10.0;
  float ylen = *_dy * 10.0;
  float len = sqrt(xlen*xlen + ylen*ylen);
  if( len > 1.5 ){
    // streaking starts
    
    float color = 1.0 / sqrt(len);
    
    glBegin(GL_LINES);
      for(int i=0;i<maxStars;i++){
        glColor3f(stars[i][BRIGHT]*color , stars[i][BRIGHT]*color , stars[i][BRIGHT]*color);
        glVertex2f(stars[i][XX] , stars[i][YY]);
        glVertex2f(stars[i][XX]+xlen , stars[i][YY]+ylen);
      }
    glEnd();
  }else{
    
    // non-streaking stars
    glBegin(GL_POINTS);
      for(int i=0;i<maxStars;++i){
        glColor3f(stars[i][BRIGHT] , stars[i][BRIGHT] , stars[i][BRIGHT]);
        glVertex2f(stars[i][XX] , stars[i][YY]);
      }
    glEnd();
  }
}
