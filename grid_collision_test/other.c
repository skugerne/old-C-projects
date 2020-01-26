#include "main.h"
#include "font.h"
#include "object.h"

#define LINE_HEIGHT (textHieght()+2)



void initDraw(){
  SDL_WM_SetCaption("Grid Collision Test", "Grid Collision Test");
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  glClearColor(0.0 , 0.0 , 0.0 , 0.0);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glBlendFunc(GL_ONE, GL_ONE);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glViewport(0, 0, SCREEN_WID, SCREEN_HIG);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, SCREEN_WID, 0, SCREEN_HIG, -1, 1);
}



void draw(){
  ++g_frames;
  
  glClear(GL_COLOR_BUFFER_BIT);

  if( g_showGrid ){
    glColor4f(0.5,0.2,0,0);
    glBegin(GL_LINES);
      for(int coord=DRAW_OFFSET_Y;coord<=(DRAW_OFFSET_Y+NUM_SECTORS_HIG*SECTOR_SIZE);coord+=SECTOR_SIZE){
        glVertex2i(DRAW_OFFSET_X,coord);
        glVertex2i(DRAW_OFFSET_X+NUM_SECTORS_WID*SECTOR_SIZE,coord);
      }
      for(int coord=DRAW_OFFSET_X;coord<=(DRAW_OFFSET_X+NUM_SECTORS_WID*SECTOR_SIZE);coord+=SECTOR_SIZE){
        glVertex2i(coord,DRAW_OFFSET_Y);
        glVertex2i(coord,DRAW_OFFSET_Y+NUM_SECTORS_HIG*SECTOR_SIZE);
      }
    glEnd();
    glColor4f(0.5,0.5,0,0);
    glBegin(GL_LINE_LOOP);
      glVertex2i(DRAW_OFFSET_X,DRAW_OFFSET_Y);
      glVertex2i(DRAW_OFFSET_X,DRAW_OFFSET_Y+WORLD_HIG);
      glVertex2i(DRAW_OFFSET_X+WORLD_WID,DRAW_OFFSET_Y+WORLD_HIG);
      glVertex2i(DRAW_OFFSET_X+WORLD_WID,DRAW_OFFSET_Y);
    glEnd();
  }
  
  object_t *oPtr = g_objects;
  while( oPtr ){
    drawObject_t(oPtr);
    oPtr = oPtr->next;
  }
  
  glEnable(GL_BLEND);
  
  char buffer[100];
  
  setTextProps(0.1,1.0,0.7,"nova",18);
  setTextStart(COLUMN_1_X,TEXT_SCREEN_TOP);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"FPS: %.1f",g_fps);
  renderText(buffer);
  
  setTextStart(COLUMN_1_X,TEXT_SCREEN_TOP - LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"IPS: %.1f",g_ups);
  renderText(buffer);
  
  {
    list_stats_t stats;
    listStats(&stats);
    
    setTextProps(0.1,0.7,1.0,"nova",18);
    setTextStart(COLUMN_2_X,TEXT_SCREEN_TOP);
    setTextJust(TEXT_LEFT,TEXT_BOTTOM);
    sprintf(buffer,"Num: %u",stats.numObj);
    renderText(buffer);
    
    setTextStart(COLUMN_2_X,TEXT_SCREEN_TOP - 1*LINE_HEIGHT);
    setTextJust(TEXT_LEFT,TEXT_BOTTOM);
    sprintf(buffer,"KE: %.2f",stats.kineticEng);
    renderText(buffer);
    
    setTextStart(COLUMN_2_X,TEXT_SCREEN_TOP - 2*LINE_HEIGHT);
    setTextJust(TEXT_LEFT,TEXT_BOTTOM);
    sprintf(buffer,"MoX: %0.02f",stats.inertiaX);
    renderText(buffer);
    
    setTextStart(COLUMN_2_X,TEXT_SCREEN_TOP - 3*LINE_HEIGHT);
    setTextJust(TEXT_LEFT,TEXT_BOTTOM);
    sprintf(buffer,"MoY: %0.02f",stats.inertiaY);
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
  
  setTextProps(0.8,0.8,0.5,"min",14);
  setTextStart(COLUMN_3_X,TEXT_SCREEN_TOP);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_showGrid )
    sprintf(buffer,"F1: hide grid");
  else
    sprintf(buffer,"F1: show grid");
  renderText(buffer);
  
  setTextStart(COLUMN_3_X,TEXT_SCREEN_TOP - LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F10: add objects");
  renderText(buffer);
  
  setTextStart(COLUMN_4_X,TEXT_SCREEN_TOP);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F2: - radius");
  renderText(buffer);
  
  setTextStart(COLUMN_4_X,TEXT_SCREEN_TOP - LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F3: + radius");
  renderText(buffer);
  
  setTextStart(COLUMN_4_X,TEXT_SCREEN_TOP - 2*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F4: - speed");
  renderText(buffer);
  
  setTextStart(COLUMN_4_X,TEXT_SCREEN_TOP - 3*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"F5: + speed");
  renderText(buffer);
  
  setTextStart(COLUMN_5_X,TEXT_SCREEN_TOP);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_worldGrav )
    sprintf(buffer,"F6: no world gravity");
  else
    sprintf(buffer,"F6: world gravity");
  renderText(buffer);
  
  setTextStart(COLUMN_5_X,TEXT_SCREEN_TOP - LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"(gravity angle = %d degrees)", (int)(g_worldGravDir * 360.0 / (M_PI * 2.0)));
  renderText(buffer);
  
  setTextStart(COLUMN_5_X,TEXT_SCREEN_TOP - 2*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"W: gravity to top (270 deg)");
  renderText(buffer);
  
  setTextStart(COLUMN_5_X,TEXT_SCREEN_TOP - 3*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"A: gravity to left (0 deg)");
  renderText(buffer);
  
  setTextStart(COLUMN_5_X,TEXT_SCREEN_TOP - 4*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"S: gravity to bottom (90 deg)");
  renderText(buffer);
  
  setTextStart(COLUMN_5_X,TEXT_SCREEN_TOP - 5*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"D: gravity to right (180 deg)");
  renderText(buffer);
  
  setTextStart(COLUMN_5_X,TEXT_SCREEN_TOP - 6*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"Q: rotate gravity counter-clockwise");
  renderText(buffer);
  
  setTextStart(COLUMN_5_X,TEXT_SCREEN_TOP - 7*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  sprintf(buffer,"E: rotate gravity clockwise");
  renderText(buffer);
  
  setTextStart(COLUMN_6_X,TEXT_SCREEN_TOP);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_gravStar )
    sprintf(buffer,"F7: no gravity to star");
  else
    sprintf(buffer,"F7: gravity to star");
  renderText(buffer);
  
  setTextStart(COLUMN_6_X,TEXT_SCREEN_TOP - 1*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_elasticCollisions )
    sprintf(buffer,"F8: inelastic collisions");
  else
    sprintf(buffer,"F8: elastic collisions");
  renderText(buffer);
  
  setTextStart(COLUMN_6_X,TEXT_SCREEN_TOP - 2*LINE_HEIGHT);
  setTextJust(TEXT_LEFT,TEXT_BOTTOM);
  if( g_borderType == BORDER_BOUNCE )
    sprintf(buffer,"F9: border wrap");
  else if( g_borderType == BORDER_WRAP )
    sprintf(buffer,"F9: no border");
  else
    sprintf(buffer,"F9: border bounce");
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
      g_worldGrav = !g_worldGrav;
      break;
    case 288:       // F7
      g_gravStar = !g_gravStar;
      setStar();
      break;
    case 289:       // F8
      g_elasticCollisions = !g_elasticCollisions;
      setStar();
      break;
    case 290:       // F9
      if( g_borderType == BORDER_BOUNCE ) g_borderType = BORDER_WRAP;
      else if( g_borderType == BORDER_WRAP ) g_borderType = BORDER_OPEN;
      else g_borderType = BORDER_BOUNCE;
      break;
    case 291:       // F10
      addSomeObjects(10);
      break;
    case 119:       // w
      g_worldGravDir = 270.0 * 2.0 * M_PI / 360.0;
      break;
    case 97:       // a
      g_worldGravDir = 0.0;
      break;
    case 115:       // s
      g_worldGravDir = 90.0 * 2.0 * M_PI / 360.0;
      break;
    case 100:       // d
      g_worldGravDir = 180.0 * 2.0 * M_PI / 360.0;
      break;
    case 113:       // q
      g_worldGravDir -= 15.0 * 2.0 * M_PI / 360.0;
      if(g_worldGravDir < 0.0) g_worldGravDir += 2.0 * M_PI;
      break;
    case 101:       // e
      g_worldGravDir += 15.0 * 2.0 * M_PI / 360.0;
      if(g_worldGravDir > 2.0 * M_PI) g_worldGravDir -= 2.0 * M_PI;
      break;
    case 27:        // esc
      setRunning(false);
      break;
    default:
      fprintf(stdout,"Unsupported keycode: %d.\n",keycode);
      break;
  }
}
