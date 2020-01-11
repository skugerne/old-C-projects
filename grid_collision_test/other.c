#include "main.h"
#include "font.h"
#include "object.h"



void initDraw(){
  SDL_WM_SetCaption("Grid Collision Test", "Grid Collision Test");
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  glClearColor(0.0 , 0.0 , 0.0 , 0.0);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glBlendFunc(GL_ONE, GL_ONE);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glViewport(0, 0, 800, 600);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,800,0,600,-1,1);
}



void draw(){
  ++g_frames;
  
  glClear(GL_COLOR_BUFFER_BIT);
  
  object_t *oPtr = g_objects;
  while( oPtr ){
    drawObject_t(oPtr);
    oPtr = oPtr->next;
  }
  
  glEnable(GL_BLEND);
  
  char buffer[100];
  
  setTextProps(0.1,1.0,0.7,"nova",18);
  setTextStart(5.0,600.0);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"FPS: %.1f",g_fps);
  renderText(buffer);
  
  setTextStart(5.0,600.0 - textHieght());
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"IPS: %.1f",g_ups);
  renderText(buffer);
  
  {
    list_stats_t stats;
    listStats(&stats);
    
    setTextProps(0.1,0.7,1.0,"nova",18);
    setTextStart(135.0,600.0);
    setTextJust(TEXT_LEFT,TEXT_BOTTOM);
    sprintf(buffer,"NO: %u",stats.numObj);
    renderText(buffer);
    
    setTextStart(135.0,600.0 - 1*textHieght());
    setTextJust(TEXT_LEFT,TEXT_BOTTOM);
    sprintf(buffer,"KE: %.2f",stats.kineticEng);
    renderText(buffer);
    
    setTextStart(135.0,600.0 - 2*textHieght());
    setTextJust(TEXT_LEFT,TEXT_BOTTOM);
    sprintf(buffer,"MoX: %.2f",stats.inertiaX);
    renderText(buffer);
    
    setTextStart(135.0,600.0 - 3*textHieght());
    setTextJust(TEXT_LEFT,TEXT_BOTTOM);
    sprintf(buffer,"MoY: %.2f",stats.inertiaY);
    renderText(buffer);
    
    if( g_showCM ){
      // show the center of mass
      
      glColor4f(1.0,0.5,0.0,0.4);
      glBegin(GL_LINES);
        glVertex2f(stats.centerMassX+25,stats.centerMassY+25);
        glVertex2f(stats.centerMassX-25,stats.centerMassY-25);
        glVertex2f(stats.centerMassX+25,stats.centerMassY-25);
        glVertex2f(stats.centerMassX-25,stats.centerMassY+25);
      glEnd();
      
      setTextProps(1.0,0.5,0.0,"min",24);
      setTextStart(stats.centerMassX,stats.centerMassY);
      setTextJust(TEXT_CENTER,TEXT_CENTER);
      renderText("CM");
    }
  }
  
  setTextProps(0.8,0.8,0.5,"min",12);
  setTextStart(265.0,600.0);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_showGrid )
    sprintf(buffer,"F1: hide grid");
  else
    sprintf(buffer,"F1: show grid");
  renderText(buffer);
  
  setTextStart(395.0,600.0);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F2: - radius");
  renderText(buffer);
  
  setTextStart(395.0,600.0 - textHieght());
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F3: + radius");
  renderText(buffer);
  
  setTextStart(395.0,600.0 - 2*textHieght());
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F4: - speed");
  renderText(buffer);
  
  setTextStart(395.0,600.0 - 3*textHieght());
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F5: + speed");
  renderText(buffer);
  
  setTextStart(525.0,600.0);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_gravBot )
    sprintf(buffer,"F6: no gravity to bottom");
  else
    sprintf(buffer,"F6: gravity to bottom");
  renderText(buffer);
  
  setTextStart(525.0,600.0 - textHieght());
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_gravStar )
    sprintf(buffer,"F7: no gravity to star");
  else
    sprintf(buffer,"F7: gravity to star");
  renderText(buffer);
  
  setTextStart(525.0,600.0 - 2*textHieght());
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_borderType == BORDER_BOUNCE )
    sprintf(buffer,"F8: border wrap");
  else if( g_borderType == BORDER_WRAP )
    sprintf(buffer,"F8: no border");
  else
    sprintf(buffer,"F8: border bounce");
  renderText(buffer);
  
  glDisable(GL_BLEND);
  
  SDL_GL_SwapBuffers();
}



void update(unsigned iterations){
  for(unsigned i=0;i<iterations;++i){
    ++g_updates;     // reset from time to time
    ++g_timestamp;   // not reset
    
    object_t *oPtr = g_objects;
    while( oPtr ){
      updateObject_t(oPtr);
      
      // checks for collisions and places object in grid (if it survives cols)
      engridObject_t(oPtr);
      
      if( oPtr->alive ){
        oPtr = oPtr->next;
      }else{
        object_t *tmp = oPtr;
        oPtr = oPtr->next;
        
        if( tmp->prev )
          tmp->prev->next = tmp->next;
        else if( tmp->next )
          tmp->next->prev = NULL;
        if( tmp->next )
          tmp->next->prev = tmp->prev;
        else if( tmp->prev )
          tmp->prev->next = NULL;
        
        delete tmp;
      }
    }
  }
}



void handleKeyDown(SDL_KeyboardEvent *key){
  int keycode = key->keysym.sym;
  
  switch ( keycode ) {
    case ' ':
      cancelMomentum();
      break;
    case 282:       // F1
      g_showGrid = !g_showGrid;
      break;
    case 283:       // F2
      radiusChange(0.9);
      break;
    case 284:       // F3
      radiusChange(1.111111);
      break;
    case 285:       // F4
      speedChange(0.75);
      break;
    case 286:       // F5
      speedChange(1.1333333);
      break;
    case 287:       // F6
      g_gravBot = !g_gravBot;
      break;
    case 288:       // F7
      g_gravStar = !g_gravStar;
      setStar();
      break;
    case 289:       // F8
      if( g_borderType == BORDER_BOUNCE ) g_borderType = BORDER_WRAP;
      else if( g_borderType == BORDER_WRAP ) g_borderType = BORDER_OPEN;
      else g_borderType = BORDER_BOUNCE;
      break;
    case 27:        // esc
      setRunning(false);
      break;
  }
}
