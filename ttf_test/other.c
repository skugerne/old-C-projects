#include "main.h"
#include "font.h"



struct display_t {
  float x,y;
  float cx,cy;
  float angle, dang;
  float rot, drot;
  float dist;
  float r,g,b,a;
};

struct fontdisp_t {
  char name[10];
  unsigned size;
};

#define NUM_RECT 3
#ifdef FONT_DEBUG
#define NUM_TEXT 0
#define NUM_RAND_TEXT 0
#define NUM_PS_TEXT 1
#else
#define NUM_TEXT 64
#define NUM_RAND_TEXT 32
#define NUM_PS_TEXT 32
#endif

display_t rectangles[NUM_RECT];
display_t strings[NUM_TEXT];
fontdisp_t strings_ext[NUM_TEXT];
display_t r_strings[NUM_RAND_TEXT];
fontdisp_t r_strings_ext[NUM_RAND_TEXT];
display_t ps_strings[NUM_PS_TEXT];
fontdisp_t ps_strings_ext[NUM_PS_TEXT];



void initDisplay_t(display_t *dPtr){
  dPtr->cx = 200.0 + 400.0 * (float)rand() / (RAND_MAX+1.0);
  dPtr->cy = 150.0 + 300.0 * (float)rand() / (RAND_MAX+1.0);
  dPtr->angle = 2*M_PI * (float)rand() / (RAND_MAX+1.0);
  dPtr->dang = 0.0001 + 0.0005 * (float)rand() / (RAND_MAX+1.0);
  dPtr->rot = 360.0 * (float)rand() / (RAND_MAX+1.0);
  dPtr->drot = -0.360 + 0.720 * (float)rand() / (RAND_MAX+1.0);
  
  #ifndef FONT_DEBUG
  dPtr->dist = 100.0 + 100.0 * (float)rand() / (RAND_MAX+1.0);
  #else
  dPtr->dist = 10.0;
  #endif
  
  dPtr->r = 0.2 + 0.8 * (float)rand() / (RAND_MAX+1.0);
  dPtr->g = 0.2 + 0.8 * (float)rand() / (RAND_MAX+1.0);
  dPtr->b = 0.2 + 0.8 * (float)rand() / (RAND_MAX+1.0);
  dPtr->a = 0.5 * (float)rand() / (RAND_MAX+1.0);
}



void initFontdisp_t(fontdisp_t *fPtr){
  int choose = 1 + (int)(5.0 * (float)rand() / (RAND_MAX+1.0));
  switch( choose ){
    case 1:
      strcpy(fPtr->name,"chow");
      break;
    case 2:
      strcpy(fPtr->name,"dirty");
      break;
    case 3:
      strcpy(fPtr->name,"lcd");
      break;
    case 4:
      strcpy(fPtr->name,"min");
      break;
    case 5:
      strcpy(fPtr->name,"nova");
      break;
    default:
      fprintf(stderr,"ERROR - Invalid option at initFontdisp_t.\n");
  }
  
  // select a font size with a tendency towards small
  fPtr->size = 18 + 2 * (int)(36.0 * (float)rand() / (RAND_MAX+1.0));
  choose = 1 + (int)(32.0 * (float)rand() / (RAND_MAX+1.0));
  if( choose == 1 )
    fPtr->size *= 2;
  else if( choose > 3 )
    fPtr->size /= 2;
  
  #ifdef FONT_DEBUG
  fPtr->size *= 4;
  fprintf(stderr,"Selected font '%s' and size %d.\n",fPtr->name,fPtr->size);
  #endif
}



void updateDisplay_t(display_t *dPtr){
  dPtr->angle += dPtr->dang;
  if( dPtr->angle > 2*M_PI )
    dPtr->angle -= 2*M_PI;
    
  dPtr->rot += dPtr->drot;
  if( dPtr->rot > 360 )
    dPtr->rot -= 360;
  
  dPtr->x = dPtr->cx;
  dPtr->x -= sin(dPtr->angle) * dPtr->dist;
  dPtr->x += cos(dPtr->angle) * dPtr->dist;
  dPtr->y = dPtr->cy;
  dPtr->y += sin(dPtr->angle) * dPtr->dist;
  dPtr->y += cos(dPtr->angle) * dPtr->dist;
}



void initDraw(){
  SDL_WM_SetCaption("TTF Font Test", "TTF Font Test");
  
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
  
  srand(time(0));
  
  for(int i=0;i<NUM_RECT;++i)
    initDisplay_t(&rectangles[i]);
  
  for(int i=0;i<NUM_TEXT;++i){
    initDisplay_t(&strings[i]);
    initFontdisp_t(&strings_ext[i]);
  }
  
  for(int i=0;i<NUM_RAND_TEXT;++i){
    initDisplay_t(&r_strings[i]);
    initFontdisp_t(&r_strings_ext[i]);
  }
  
  for(int i=0;i<NUM_PS_TEXT;++i){
    initDisplay_t(&ps_strings[i]);
    initFontdisp_t(&ps_strings_ext[i]);
    ps_strings_ext[i].size /= 2;
    ps_strings_ext[i].size += 4;
  }
  
  // get coordinates initialized, one way or anyother
  update(1);
}



void draw(){
  char buffer[100];
  
  ++g_frames;
  
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);
  
  for(int i=0;i<NUM_RECT;++i){
    glColor4f(rectangles[i].r,rectangles[i].g,rectangles[i].b,rectangles[i].a);
    glBegin(GL_QUADS);
      glVertex2f(-100+rectangles[i].x,-100+rectangles[i].y);
      glVertex2f(100+rectangles[i].x,-100+rectangles[i].y);
      glVertex2f(100+rectangles[i].x,100+rectangles[i].y);
      glVertex2f(-100+rectangles[i].x,100+rectangles[i].y);
    glEnd();
  }
  
  for(int i=0;i<NUM_TEXT;++i){
    setTextProps(strings[i].r,strings[i].g,strings[i].b,strings_ext[i].name,strings_ext[i].size);
    setTextStart(strings[i].x,strings[i].y,strings[i].rot);
    #ifdef FONT_DEBUG
    setTextJust(TEXT_CENTER,g_textVert);
    #else
    setTextJust(TEXT_CENTER,TEXT_CENTER);
    #endif
    renderText("Damn I'm sexy!");
  }
  
  for(int i=0;i<NUM_RAND_TEXT;++i){
    sprintf(buffer,"X: %.04f  Y: %.04f",r_strings[i].x,r_strings[i].y);
    setTextProps(r_strings[i].r,r_strings[i].g,r_strings[i].b,r_strings_ext[i].name,r_strings_ext[i].size);
    setTextStart(r_strings[i].x,r_strings[i].y);
    #ifdef FONT_DEBUG
    setTextJust(TEXT_CENTER,g_textVert);
    #else
    setTextJust(TEXT_CENTER,TEXT_CENTER);
    #endif
    renderText(buffer);
  }
  
  sprintf(buffer,"FPS: %.1f  UPS: %.1f",g_fps,g_ups);
  for(int i=0;i<NUM_PS_TEXT;++i){
    setTextProps(ps_strings[i].r,ps_strings[i].g,ps_strings[i].b,ps_strings_ext[i].name,ps_strings_ext[i].size);
    setTextStart(ps_strings[i].x,ps_strings[i].y);
    #ifdef FONT_DEBUG
    setTextJust(TEXT_CENTER,g_textVert);
    #else
    setTextJust(TEXT_CENTER,TEXT_CENTER);
    #endif
    renderText(buffer);
  }
  
  glDisable(GL_BLEND);
  SDL_GL_SwapBuffers();
}



void update(unsigned iterations){
  for(unsigned i=0;i<iterations;++i){
    ++g_updates;
    
    for(int i=0;i<NUM_RECT;++i)
      updateDisplay_t(&rectangles[i]);
    
    for(int i=0;i<NUM_TEXT;++i)
      updateDisplay_t(&strings[i]);
    
    for(int i=0;i<NUM_RAND_TEXT;++i)
      updateDisplay_t(&r_strings[i]);
    
    for(int i=0;i<NUM_PS_TEXT;++i)
      updateDisplay_t(&ps_strings[i]);
  }
}



void newFonts(){
  for(int i=0;i<NUM_TEXT;++i)
    initFontdisp_t(&strings_ext[i]);
  
  for(int i=0;i<NUM_RAND_TEXT;++i)
    initFontdisp_t(&r_strings_ext[i]);
  
  for(int i=0;i<NUM_PS_TEXT;++i)
    initFontdisp_t(&ps_strings_ext[i]);
}
