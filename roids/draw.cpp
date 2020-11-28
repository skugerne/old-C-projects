#include "main.hpp"



void initDraw(){
  SDL_WM_SetCaption("The Roids Project 5.0", "The Roids Project 5.0");
  
  // call the function that does all the work with default resolution
  initDraw(RES_1024x768);
}



// initialize graphics with given resolution
void initDraw(resolutiontype res){
  switch (res) {
    case RES_640x480:
      _physicalX = 640;
      _physicalY = 480;
      break;
    case RES_800x600:
      _physicalX = 800;
      _physicalY = 600;
      break;
    case RES_1024x768:
      _physicalX = 1024;
      _physicalY = 768;
      break;
    default:
      #ifdef DEBUG
      fprintf(stderr,"WARNG - unrecognised value for res at initDraw(resolutiontype).\n");
      #endif
      return;
  }
  
  //-----------------------------------------------------------------------------
  // many operating systems have their OpenGL textures whiped out by view changes
  
  // delete font display lists
  glDeleteLists( _fontDisLisStart, _numberOfFonts * 256 );
  
  // Clean up our textures
  glDeleteTextures( NUM_TEXTURES, &_texture[0] );
  
  //-----------------------------------------------------------------------------
  // continue on with setup of new window size
  
  _screen = SDL_SetVideoMode(_physicalX, _physicalY, 16, _videoFlags);
  if(!_screen){
    fprintf(stderr,"ERROR - Failed to set GL video mode: %s.\n",SDL_GetError());
    SDL_Quit();
    exit(1);
  }
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  glClearColor(0.0 , 0.0 , 0.0 , 0.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  
  glViewport(0, 0, _physicalX, _physicalY);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-_virtualPosX,_virtualPosX,-_virtualPosY,_virtualPosY,-1,1);
  
  // for reference, this is the other blend mode that I find useful
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  //-----------------------------------------------------------------------------
  // reloading the textures
  
  // load in textures (including fonts), then process them as needed
  if(!loadFontTexture() || !loadTextures()){
    fprintf(stderr,"ERROR - Failed to load textures (fatal).\n");
    exit(1);
  }
}



void drawGrids(){
  
  // 2 * _virtualPosY should be the largest dimention we need
  // (largest dimention is complicated by screen rotation)
  double distance = _virtualPosY * 3 / 2;
  
  double upper, lower;

  // ********************************************************************* 
  // sector lines
  
  if(_option1){
    glColor3f(0.3,0.3,0.2);
    
    double coord = 0 - (int)*_x % SECTOR_SIZE;
    while(coord > -distance) coord -= SECTOR_SIZE;
    if(0 - *_x > coord) coord = -*_x;
    
    upper = distance;
    lower = -distance;
    
    // bottom line end trim
    if(*_y < distance) lower = -*_y;
    
    // top line end trim
    if(MAX_COORDINATE - *_y < distance) upper = MAX_COORDINATE - *_y;
    
    // draw the vertical lines
    while(coord < distance && MAX_COORDINATE - *_x > coord){
      glBegin(GL_LINES);
        glVertex2f(coord , upper);
        glVertex2f(coord , lower);
      glEnd();
      coord += SECTOR_SIZE;
    }
    
    coord = 0 - (int)*_y % SECTOR_SIZE;
    while(coord > -distance) coord -= SECTOR_SIZE;
    if(0 - *_y > coord) coord = -*_y;
    
    upper = distance;
    lower = -distance;
    
    // left line end trim
    if(*_x < distance) lower = -*_x;
    
    // right line end trim
    if(MAX_COORDINATE - *_x < distance) upper = MAX_COORDINATE - *_x;
    
    // draw the horizontal lines
    while(coord < distance && MAX_COORDINATE - *_y > coord){
      glBegin(GL_LINES);
        glVertex2f(upper , coord);
        glVertex2f(lower , coord);
      glEnd();
      coord += SECTOR_SIZE;
    }
  }
  
  // ********************************************************************* 
  // draw map boundaries
  
  upper = distance;
  lower = -distance;
  
  // bottom line end trim
  if(*_y < distance) lower = -*_y;
    
  // top line end trim
  if(MAX_COORDINATE - *_y < distance) upper = MAX_COORDINATE - *_y;
  
  // left line
  if(*_x < distance){
    glColor3f(1,1,1);
    glBegin(GL_LINES);
      glVertex2f(-*_x, upper);
      glVertex2f(-*_x, lower);
    glEnd();
  }
  
  // right line
  if(MAX_COORDINATE - *_x < distance){
    glColor3f(1,1,1);
    glBegin(GL_LINES);
      glVertex2f(MAX_COORDINATE - *_x, upper);
      glVertex2f(MAX_COORDINATE - *_x, lower);
    glEnd();
  }
  
  upper = distance;
  lower = -distance;
  
  // left line end trim
  if(*_x < distance) lower = -*_x;
    
  // right line end trim
  if(MAX_COORDINATE - *_x < distance) upper = MAX_COORDINATE - *_x;
  
  // bottom line
  if(*_y < distance){
    glColor3f(1,1,1);
    glBegin(GL_LINES);
      glVertex2f(upper, -*_y);
      glVertex2f(lower, -*_y);
    glEnd();
  }
  
  // top line
  if(MAX_COORDINATE - *_y < distance){
    glColor3f(1,1,1);
    glBegin(GL_LINES);
      glVertex2f(upper, MAX_COORDINATE - *_y);
      glVertex2f(lower, MAX_COORDINATE - *_y);
    glEnd();
  }

}



void draw(){
  #ifdef DEBUG
  fprintf(stderr,"Draw.\n");
  
  if(!_player){
    fprintf(stderr,"ERROR - starting draw without _player.\n");
    return;
  }
  #endif
  
  // used primarily for the _fps count
  ++_frames;
  
  // *********************************************************************
  // viewpoint rotation
  
  if(_viewMode == VIEW_WORLD_AXIS_ALIGNED){
    _goalSceenRotation = 0;
  }else if(_viewMode == VIEW_PLAYER_AXIS_ALIGNED){
    _goalSceenRotation = -*_a + 90;
  }else{
    if(*_dy != 0 || *_dx != 0){
      _goalSceenRotation = atan(*_dy / *_dx);
      if(*_dx < 0) _goalSceenRotation += 3.141593;
      if(_goalSceenRotation < 0) _goalSceenRotation += 6.283185;
        
      _goalSceenRotation = ( _goalSceenRotation / 6.28 ) * -360;
      
      if(_viewMode == VIEW_PLAYER_MOTION_ALIGNED){
        _goalSceenRotation += 90;
      }
    }
  }
  
  glClear(GL_COLOR_BUFFER_BIT);
  
  if(_sceenRotation != 0.0){
    glPushMatrix();
    glRotatef(_sceenRotation , 0.0 , 0.0 , 1.0);
  }
  
  // *********************************************************************
  // deal with the stars
  
  #ifdef DEBUG_DRAW
  fprintf(stderr,"Drawing _stars next.\n");
  #endif
  
  {
    int todo;
    if( _fps > 50.0 )
      todo = NUM_STARFIELDS;
    else
      todo = (int)((float)(NUM_STARFIELDS * _fps) / 50.0);
    if( todo < 1 )
      todo = 1;
    
    for(int i=0;i<todo;++i)
      _stars[i]->draw();
  }
    
  // ********************************************************************* 
  // grids and map boundaries
  
  #ifdef DEBUG_DRAW
  fprintf(stderr,"Drawing grids next.\n");
  #endif
  drawGrids();
  
  // *********************************************************************
  // particle systems
  
  #ifdef DEBUG_DRAW
  fprintf(stderr,"Drawing particles next.\n");
  #endif
  drawDust();
  drawFlame();
  
  // ********************************************************************* 
  // drawing viewable objects
  
  #ifdef DEBUG_DRAW
  fprintf(stderr,"Drawing objects next.\n");
  #endif
  
  objecttype *oPtr = _objects;
  
  while(oPtr != NULL){
    
    if( oPtr->viewable( oPtr->distanceFrom(_player) ) ){
      oPtr->draw();
    }
    
    oPtr = oPtr->getNext();
  }
  
  // a bit crude ... drawing the star
  if(_starCore->distanceFrom(_player) < 2 * STAR_RAYS + _virtualPosX){
    _starCore->draw();
  }
  
  // *********************************************************************
  // pop us out of the potentially rotated world
  
  if(_sceenRotation != 0.0){
    glPopMatrix();
  }
  
  // ********************************************************************* 
  // GUI from the _player
  
  #ifdef DEBUG_DRAW
  fprintf(stderr,"Drawing player stuff next.\n");
  #endif
  _player->draw();
  
  // ********************************************************************* 
  // status
  
  #ifdef DEBUG_DRAW
  fprintf(stderr,"Drawing status stuff next.\n");
  #endif
  
  char buf[20];
  glEnable(GL_BLEND);
  glColor4f(.8,.8,0,.8);
  
  // go to our 1024x768 simple world
  setOverlayMode();
  
  sprintf(buf,"FPS: %.1f",_fps);
  printStringToRight(FONT_SMALL, false, &buf[0], 5, OVERLAY_Y-30);
  sprintf(buf,"UPS: %.1f",_ups);
  printStringToRight(FONT_SMALL, false, &buf[0], 5, OVERLAY_Y-50);
  
  sprintf(buf,"Objects: %d",_numObj);
  printStringToLeft(FONT_SMALL, false, &buf[0], OVERLAY_X-5, OVERLAY_Y-30);
  sprintf(buf,"Time: %ds",_time/1000);
  printStringToLeft(FONT_SMALL, false, &buf[0], OVERLAY_X-5, OVERLAY_Y-50);
  sprintf(buf,"Level: %d",_level);
  printStringToLeft(FONT_SMALL, false, &buf[0], OVERLAY_X-5, OVERLAY_Y-90);
  
  // back to the real-world coordinate system
  setNormalMode();
  
  glDisable(GL_BLEND);
  
  // ********************************************************************* 
  // menu
  
  // menu decides if it actually needs to draw anything
  glEnable(GL_BLEND);
  drawMenu();
  glDisable(GL_BLEND);
  
  // *********************************************************************
  // flip buffers
  
  SDL_GL_SwapBuffers();
  
  #ifdef DEBUG
  fprintf(stderr,"End draw.\n");
  #endif
}
