#include "main.hpp"



enum menumodetype { MM__PRE_START,
                    MM_START,
                    MM__START_TO_GAME,
                    MM_SOLO,
                    MM__START_TO_SOLO,
                    MM__SOLO_TO_START,
                    MM__SOLO_TO_GAME,
                    MM_HOST,
                    MM__START_TO_HOST,
                    MM__HOST_TO_START,
                    MM_JOIN,
                    MM__START_TO_JOIN,
                    MM__JOIN_TO_START,
                    MM_PLAYER,
                    MM__START_TO_PLAYER,
                    MM__PLAYER_TO_START,
                    MM_SYNCRO,
                    MM__HOST_TO_SYNCRO,
                    MM__JOIN_TO_SYNCRO,
                    MM__SYNCRO_TO_START,
                    MM__SYNCRO_TO_GAME,
                    MM_ERROR,
                    MM__SYNCRO_TO_ERROR,
                    MM__ERROR_TO_START,
                    MM_NUMBER_OF_MENU_MODES,
                    MM_MENU_IS_OFF } menumode;
                    // MM_NUMBER_OF_MENU_MODES is not a valid menu mode

// pre-declaration for fcn that calls the init fcn's of the modes
void switchToState();
                    
enum functionmodetype { FM_INIT,
                        FM_DRAW,
                        FM_UPDATE,
                        FM_KEYS };

// an array of pointers to funtions
bool (*menuFuncs[MM_NUMBER_OF_MENU_MODES])(functionmodetype,int);

// ----------------------------------------------------------------------
// behold!
// here is a large mess, all local-globals shared by the whole menu system ...
// ... (which isn't as bad as that may sound because only one menu runs at ...
// ... once anyway)

// some day I should neaten these into nice structs with meaningful names
// for example, a struct with "actual" and "goal" float parts
float leftEdge, rightEdge, topEdge, bottomEdge;
float goalLeftEdge, goalRightEdge, goalTopEdge, goalBottomEdge;
float horizEdgeMoveRate, vertEdgeMoveRate, titleMoveRateX, titleMoveRateY;
float titleX, titleY, goalTitleX, goalTitleY;

// there are actually other fade variables that are used as locals as well
// it may be better to simply treat them all as globals and do away with ...
// ... parameters to the "text" functions
// some day I should neaten these into nice structs with meaningful names
float mainFade, scndFade, mainFadeRate, scndFadeRate;

// limited-use variables for IP-address handling
int selected;
char *serverIP;
bool errrFlashing;
Uint errStart;

// limited-use variables used for "syncro" status reporting
int oDone;
objecttype *oPtr;

// ----------------------------------------------------------------------



// go out and read .servers to allow server IP autocomplete
void readServersList(){


}



// write out the .servers file
void writeServersList(){


}



bool verifyIP(char *ip){
  if(!ip){
    #ifdef DEBUG_NET
    fprintf(stderr,"ERROR - NULL IP passed to verifyIP().\n");
    #endif
    return false;
  }
  
  Uchar buf[16];
  
  int len = 0;
  while(ip[len]){
    
    if(len > 15){
      #ifdef DEBUG_NET
      fprintf(stderr,"ERROR - overlong IP passed to verifyIP().\n");
      #endif
      return false;
    }
    buf[len] = ip[len] - 48;
    if(buf[len] > 9 && buf[len] != 254){
      #ifdef DEBUG_NET
      fprintf(stderr,"ERROR - illegal character %d '%d' passed to verifyIP().\n",len,buf[len]);
      #endif
      return false;
    }
    
    ++len;
  }
  
  if(len < 15){
    #ifdef DEBUG_NET
    fprintf(stderr,"ERROR - short IP passed to verifyIP().\n");
    #endif
    return false;
  }
  
  for(int i=0;i<4;++i){
    int val = buf[4*i] * 100 + buf[4*i+1] * 10 + buf[4*i+2];
    if(val > 255 || val < 0){
      #ifdef DEBUG_NET
      fprintf(stderr,"WARNG - IP with '%d' passed to verifyIP().\n",val);
      fprintf(stderr,"ERROR - bad IP passed to verifyIP().\n");
      #endif
      return false;
    }
  }
  
  return true;
}


void basicMenuUpdate(){
  leftEdge -= horizEdgeMoveRate;
  rightEdge += horizEdgeMoveRate;
  bottomEdge -= vertEdgeMoveRate;
  topEdge += vertEdgeMoveRate;
  titleX += titleMoveRateX;
  titleY += titleMoveRateY;
  scndFade -= scndFadeRate;       // 0.1 is half second (1.0 -> 0.0)
  mainFade -= mainFadeRate;
}



void drawMenuBackground(){
  glPushMatrix();
  
  glTranslated(OVERLAY_X / 2, OVERLAY_Y / 2, 0);
  
  int wid = (int)(rightEdge - leftEdge) / 2;
  int hig = (int)(topEdge - bottomEdge) / 2;
  
  const int texDim = 256;
  
  int fullSquaresHig = hig / texDim;
  int partSquaresHig = hig % texDim;
  int fullSquaresWid = wid / texDim;
  int partSquaresWid = wid % texDim;
  
  float partialH, partialV;
  
  // square to block out background
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0.0, 0.0, 0.0, 0.95);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
    glVertex2i( wid, hig );
    glVertex2i( -wid, hig );
    glVertex2i( -wid, -hig );
    glVertex2i( wid, -hig );
  glEnd();
  glEnable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  
  glColor4f(1.0, 1.0, 1.0, mainFade);
  glBindTexture( GL_TEXTURE_2D, _texture[14] );
  
  // in this float loop, we draw those 256x256 squares that are 100% ...
  // ... inside the menu box area
  for(int i=-fullSquaresWid;i<fullSquaresWid;++i)
    for(int j=-fullSquaresHig;j<fullSquaresHig;++j){
      glBegin(GL_QUADS);
        // bottom left
        glTexCoord2f( 0, 1 );
        glVertex2i(i * 256, j * 256);
          
        // bottom right
        glTexCoord2f( 1, 1 );
        glVertex2i((i+1) * 256, j * 256);
          
        // top right
        glTexCoord2f( 1, 0 );
        glVertex2i((i+1) * 256, (j+1) * 256);
          
        // top left
        glTexCoord2f( 0, 0 );
        glVertex2i(i * 256, (j+1) * 256);
      glEnd();
    }
  
  // in this loop, we add partial hieght yet full-width textures (top/bottom)
  partialH = (float)partSquaresHig / 256;
  for(int i=-fullSquaresWid;i<fullSquaresWid;++i){
    glBegin(GL_QUADS);
      // bottom left
      glTexCoord2f( 0, 1 );
      glVertex2i(i * 256, fullSquaresHig * 256);
        
      // bottom right
      glTexCoord2f( 1, 1 );
      glVertex2i((i+1) * 256, fullSquaresHig * 256);
        
      // top right
      glTexCoord2f( 1, 1 - partialH );
      glVertex2i((i+1) * 256, fullSquaresHig * 256 + partSquaresHig);
        
      // top left
      glTexCoord2f( 0, 1 - partialH );
      glVertex2i(i * 256, fullSquaresHig * 256 + partSquaresHig);
      
      // ************* bottom side quad ***************
      // bottom left
      glTexCoord2f( 0, partialH );
      glVertex2i(i * 256, -fullSquaresHig * 256 - partSquaresHig);
        
      // bottom right
      glTexCoord2f( 1, partialH );
      glVertex2i((i+1) * 256, -fullSquaresHig * 256 - partSquaresHig);
        
      // top right
      glTexCoord2f( 1, 0 );
      glVertex2i((i+1) * 256, -fullSquaresHig * 256);
        
      // top left
      glTexCoord2f( 0, 0 );
      glVertex2i(i * 256, -fullSquaresHig * 256);
    glEnd();
  }
  
  // in this loop, we add partial width yet full hieght textures (left/right)
  partialV = (float)partSquaresWid / 256;
  for(int i=-fullSquaresHig;i<fullSquaresHig;++i){
    glBegin(GL_QUADS);
      // bottom left
      glTexCoord2f( 0, 1 );
      glVertex2i(fullSquaresWid * 256, i * 256);
        
      // bottom right
      glTexCoord2f( partialV, 1 );
      glVertex2i(fullSquaresWid * 256 + partSquaresWid, i * 256);
        
      // top right
      glTexCoord2f( partialV, 0 );
      glVertex2i(fullSquaresWid * 256 + partSquaresWid, (i+1) * 256);
        
      // top left
      glTexCoord2f( 0, 0 );
      glVertex2i(fullSquaresWid * 256, (i+1) * 256);
      
      // ************* left side quad ***************
      // bottom left
      glTexCoord2f( 1 - partialV, 1 );
      glVertex2i(-fullSquaresWid * 256 - partSquaresWid, i * 256);
        
      // bottom right
      glTexCoord2f( 1, 1 );
      glVertex2i(-fullSquaresWid * 256, i * 256);
        
      // top right
      glTexCoord2f( 1, 0 );
      glVertex2i(-fullSquaresWid * 256, (i+1) * 256);
        
      // top left
      glTexCoord2f( 1 - partialV, 0 );
      glVertex2i(-fullSquaresWid * 256 - partSquaresWid, (i+1) * 256);
    glEnd();
  }
  
  // use partialH and partialV calculated above
  glBegin(GL_QUADS);
    // ************* upper right ***************
    // bottom left
    glTexCoord2f( 0, 1 );
    glVertex2i(fullSquaresWid * 256, fullSquaresHig * 256);
      
    // bottom right
    glTexCoord2f( partialV, 1 );
    glVertex2i(fullSquaresWid * 256 + partSquaresWid, fullSquaresHig * 256);
      
    // top right
    glTexCoord2f( partialV, 1 - partialH );
    glVertex2i(fullSquaresWid * 256 + partSquaresWid, fullSquaresHig * 256 + partSquaresHig);
      
    // top left
    glTexCoord2f( 0, 1 - partialH );
    glVertex2i(fullSquaresWid * 256, fullSquaresHig * 256 + partSquaresHig);
    
    // ************* upper left ***************
    // bottom left
    glTexCoord2f( 1 - partialV, 1 );
    glVertex2i(-fullSquaresWid * 256 - partSquaresWid, fullSquaresHig * 256);
      
    // bottom right
    glTexCoord2f( 1, 1 );
    glVertex2i(-fullSquaresWid * 256, fullSquaresHig * 256);
      
    // top right
    glTexCoord2f( 1, 1 - partialH );
    glVertex2i(-fullSquaresWid * 256, fullSquaresHig * 256 + partSquaresHig);
      
    // top left
    glTexCoord2f( 1 - partialV, 1 - partialH );
    glVertex2i(-fullSquaresWid * 256 - partSquaresWid, fullSquaresHig * 256 + partSquaresHig);
    
    // ************* lower right ***************
    // bottom left
    glTexCoord2f( 0, partialH );
    glVertex2i(fullSquaresWid * 256, -fullSquaresHig * 256 - partSquaresHig);
      
    // bottom right
    glTexCoord2f( partialV, partialH );
    glVertex2i(fullSquaresWid * 256 + partSquaresWid, -fullSquaresHig * 256 - partSquaresHig);
      
    // top right
    glTexCoord2f( partialV, 0  );
    glVertex2i(fullSquaresWid * 256 + partSquaresWid, -fullSquaresHig * 256);
      
    // top left
    glTexCoord2f( 0, 0 );
    glVertex2i(fullSquaresWid * 256, -fullSquaresHig * 256);
    
    // ************* lower left ***************
    // bottom left
    glTexCoord2f( 1 - partialV, partialH );
    glVertex2i(-fullSquaresWid * 256 - partSquaresWid, -fullSquaresHig * 256 - partSquaresHig);
      
    // bottom right
    glTexCoord2f( 1, partialH );
    glVertex2i(-fullSquaresWid * 256, -fullSquaresHig * 256 - partSquaresHig);
      
    // top right
    glTexCoord2f( 1, 0 );
    glVertex2i(-fullSquaresWid * 256, -fullSquaresHig * 256);
      
    // top left
    glTexCoord2f( 1 - partialV, 0 );
    glVertex2i(-fullSquaresWid * 256 - partSquaresWid, -fullSquaresHig * 256);
  glEnd();
  
  glPopMatrix();
  
  glColor3f(1.0, 0.9, 0.8);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_LINE_LOOP);
    glVertex2f(leftEdge,topEdge);
    glVertex2f(leftEdge,bottomEdge);
    glVertex2f(rightEdge,bottomEdge);
    glVertex2f(rightEdge,topEdge);
  glEnd();
  glEnable(GL_TEXTURE_2D);
}



// standardizes the drawing to reduce code size
void handle_START_text(float fade, bool showS, bool showH, bool showJ, bool showP){
  
  // close menu
  glPushMatrix();
  glColor4f(1.0, 0.3, 0.0, fade);
  glTranslated(417,425+37.5,0);
  glCallList(257);
  glColor4f(1.0, 1.0, 1.0, fade);
  glTranslated(26,1,0);
  glCallList(258);
  glPopMatrix();
  
  // solo game
  if(showS){
    glPushMatrix();
    glColor4f(1.0, 0.3, 0.0, fade);
    glTranslated(423,350-37.5,0);
    glCallList(259);
    glColor4f(1.0, 1.0, 1.0, fade);
    glTranslated(23,1,0);
    glCallList(260);
    glPopMatrix();
  }
      
  // host game
  if(showH){
    glPushMatrix();
    glColor4f(1.0, 0.3, 0.0, fade);
    glTranslated(423+150,350+37.5,0);
    glCallList(261);
    glColor4f(1.0, 1.0, 1.0, fade);
    glTranslated(25,0,0);
    glCallList(262);
    glPopMatrix();
  }
      
  // join game
  if(showJ){
    glPushMatrix();
    glColor4f(1.0, 0.3, 0.0, fade);
    glTranslated(426-150,350+37.5,0);
    glCallList(263);
    glColor4f(1.0, 1.0, 1.0, fade);
    glTranslated(17,0,0);
    glCallList(264);
    glPopMatrix();
  }
  
  // player
  if(showP){
    glPushMatrix();
    glColor4f(1.0, 0.3, 0.0, fade);
    glTranslated(463,200,0);
    glCallList(265);
    glColor4f(1.0, 1.0, 1.0, fade);
    glTranslated(25,-32,0);
    glCallList(266);
    glPopMatrix();
  }
  
  // quit
  glPushMatrix();
  glColor4f(1.0, 0.3, 0.0, fade);
  glTranslated(480,125,0);
  glCallList(267);
  glColor4f(1.0, 1.0, 1.0, fade);
  glTranslated(30,0,0);
  glCallList(268);
  glPopMatrix();
  
  // openGL logo
  glPushMatrix();
  glTranslated(680,114,0);
  glCallList(269);
  glPopMatrix();
  
  // SDL logo
  glPushMatrix();
  glTranslated(150,123,0);
  glCallList(271);
  glPopMatrix();
  
  glColor4f(1.0, 1.0, 0.5, fade);
  
  // title, 3 piece
  glPushMatrix();
  glTranslated(226,550,0);
  glCallList(272);
  glTranslated(225,0,0);
  glCallList(273);
  glTranslated(224,-66,0);
  glCallList(274);
  glPopMatrix();
}



// standardizes the drawing to reduce code size
void handle_SOLO_text(float fade1, float fade2){
  // title
  glPushMatrix();
  glColor4f(1.0, 0.3 + fade1 * 0.7, fade1 / 2, 1.0);
  glTranslated(titleX,titleY,0);
  glCallList(259);
  glColor4f(1.0, 1.0, 1.0 - fade1 / 2, 1.0);
  glTranslated(23,0,0);
  glCallList(260);
  glPopMatrix();
  
  glColor4f(1.0, 1.0, 1.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"Starting new single player game.", 512-160, 360);
  
  // cancel
  glColor4f(1.0, 0.3, 0.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"C", 482, 310);
  glColor4f(1.0, 1.0, 1.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"ancel", 492, 310);
}



// standardizes the drawing to reduce code size
void handle_HOST_text(float fade1, float fade2, float titleFade){
  // title
  glPushMatrix();
  glColor4f(1.0, 0.3 + fade1 * 0.7, fade1 / 2, titleFade);
  glTranslated(titleX,titleY,0);
  glCallList(261);
  glColor4f(1.0, 1.0, 1.0 - fade1 / 2, titleFade);
  glTranslated(25,0,0);
  glCallList(262);
  glPopMatrix();
  
  char str[40];
  sprintf(str,"Waiting for connection at port %d",_thisIP.port);
  int len = 0;
  while(str[len]) ++len;
  glColor4f(1.0, 1.0, 1.0, fade2);
  printStringToRight(FONT_MEDIUM,false,str, 512-5*len, 360);
  
  // cancel
  glColor4f(1.0, 0.3, 0.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"C", 482, 310);
  glColor4f(1.0, 1.0, 1.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"ancel", 492, 310);
}



// standardizes the drawing to reduce code size
// the third fade parameter is for pulsating text
void handle_JOIN_text(float fade1, float fade2, float fade3, float titleFade){
  // title
  glPushMatrix();
  glColor4f(1.0, 0.3 + fade1 * 0.7, fade1 / 2, titleFade);
  glTranslated(titleX,titleY,0);
  glCallList(263);
  glColor4f(1.0, 1.0, 1.0 - fade1 / 2, titleFade);
  glTranslated(17,0,0);
  glCallList(264);
  glPopMatrix();
      
  // ip range-to-be
  for(int i=0;i<16;++i){
    if(!errrFlashing || !(((_timestamp - errStart) / 250) % 2) ){
      if(i == selected)
        glColor4f(1.0, .3 + .7 * fade3, fade3, fade2);
      else
        glColor4f(1.0, 1.0, 1.0, fade2);
    }else{
      glColor4f(1.0, .3 + .7 * fade3, fade3, fade2);
    }
    printCharToRight(FONT_MEDIUM,false,serverIP[i], 457 + (i * 10), 360);
  }
  
  if(errrFlashing){
    if( (((_timestamp - errStart) / 250) % 2) ){
      fprintf(stderr,"Y: %d - %d = %d\n",_timestamp,errStart,_timestamp - errStart);
    }else{
      fprintf(stderr,"N: %d - %d = %d\n",_timestamp,errStart,_timestamp - errStart);
    }
  }
      
  // go
  glColor4f(1.0, 0.3, 0.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"G", 417, 360);
  glColor4f(1.0, 1.0, 1.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"o:", 427, 360);
      
  // cancel
  glColor4f(1.0, 0.3, 0.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"C", 482, 310);
  glColor4f(1.0, 1.0, 1.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"ancel", 492, 310);
}



// standardizes the drawing to reduce code size
// the third fade parameter is for pulsating text
void handle_PLAYER_text(float fade1, float fade2){
  // title
  glPushMatrix();
  glColor4f(1.0, 0.3 + fade1 * 0.7, fade1 / 2, 1.0);
  glTranslated(titleX,titleY,0);
  glCallList(265);
  glColor4f(1.0, 1.0, 1.0 - fade1 / 2, 1.0);
  glTranslated(25,-32,0);
  glCallList(266);
  glPopMatrix();
      
  char buf2[50];
  glColor4f(.3,.5,1,.8*fade2);
  
  int x = OVERLAY_X / 2 - 245;
  int y = OVERLAY_Y / 2 + 60;
  
  sprintf(buf2,"Esc: Summon menu.");
  printStringToRight(FONT_SMALL, false, &buf2[0], x, y);
  sprintf(buf2,"Cntl-Esc: Exit when escape key fails.");
  printStringToRight(FONT_SMALL, false, &buf2[0], x, y-20);
  sprintf(buf2,"F1: Toggle fullscreen.");
  printStringToRight(FONT_SMALL, false, &buf2[0], x, y-40);
  sprintf(buf2,"F2: Cycle resolution.");
  printStringToRight(FONT_SMALL, false, &buf2[0], x, y-60);
  sprintf(buf2,"F5: Toggle lines and labels.");
  printStringToRight(FONT_SMALL, false, &buf2[0], x, y-80);
  
  sprintf(buf2,"Simultaneous left and right turn is instant stop.");
  printStringToRight(FONT_SMALL, false, &buf2[0], x, y-100);
  
  glColor4f(.6,.8,1,.8*fade2);
  sprintf(buf2,"Movement: W-E-R and arrows.\n");
  printStringToRight(FONT_SMALL, false, &buf2[0], x, y-140);
  sprintf(buf2,"Fire:     Z-X and space.\n");
  printStringToRight(FONT_SMALL, false, &buf2[0], x, y-160);
  
  // cancel
  glColor4f(1.0, 0.3, 0.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"D", 477, 220);
  glColor4f(1.0, 1.0, 1.0, fade2);
  printStringToRight(FONT_MEDIUM,false,"ismiss", 487, 220);
}



// standardizes the drawing to reduce code size
void handle_SYNCRO_text(float fade){
  
  // title
  glPushMatrix();
  glColor4f(1.0, 1.0, 0.5, fade);
  glTranslated(512-89,408,0);
  glCallList(275);
  glPopMatrix();
  
  glColor4f(1.0, 1.0, 1.0, fade);
  
  // progress bar
  glColor4f(1.0, 0.0, 0.0, fade/2);
  glBegin(GL_QUADS);
    glVertex2f(512-100+20000*(float)oDone/(float)_numObj, 630-15);
    glVertex2f(512-100+20000*(float)oDone/(float)_numObj, 630+15);
    glVertex2f(512+100, 630+15);
    glVertex2f(512+100, 630-15);
  glEnd();
  glColor4f(0.0, 1.0, 0.0, fade/2);
  glBegin(GL_QUADS);
    glVertex2f(512-100, 630-15);
    glVertex2f(512-100, 630+15);
    glVertex2f(512-100+20000*(float)oDone/(float)_numObj, 630+15);
    glVertex2f(512-100+20000*(float)oDone/(float)_numObj, 630-15);
  glEnd();
  glColor4f(1.0, 1.0, 1.0, fade);
  glBegin(GL_LINE_LOOP);
    glVertex2i(512-100, 630-150);
    glVertex2i(512-100, 630+15);
    glVertex2i(512+100, 630+150);
    glVertex2i(512+100, 630-15);
  glEnd();
  
  // progress in text
  char buf[40];
  sprintf(buf,"%d of %d objects transferred.",oDone,_numObj);
  printStringCentered(FONT_MEDIUM, false, &buf[0], 512, 360);
  
  // cancel
  glColor4f(1.0, 0.3, 0.0, fade);
  printStringToRight(FONT_MEDIUM,false,"C", 482, 310);
  glColor4f(1.0, 1.0, 1.0, fade);
  printStringToRight(FONT_MEDIUM,false,"ancel", 492, 310);
}




bool handle__PRE_START(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_START_text(mainFade*mainFade*mainFade,true,true,true,true);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(mainFade >= 1.0){
        menumode = MM_START;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      leftEdge = (float)OVERLAY_X / 2;
      rightEdge = (float)OVERLAY_X / 2;
      topEdge = (float)OVERLAY_Y / 2;
      bottomEdge = (float)OVERLAY_Y / 2;
      goalLeftEdge = 100;
      goalRightEdge = 924;
      goalTopEdge = 668;
      goalBottomEdge = 100;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = (float)OVERLAY_X / 2;
      titleY = (float)OVERLAY_Y / 2;
      mainFade = 0.0;
      mainFadeRate = -0.1;
      break;
  }
  
  return false;
}



bool handle_START(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_START_text(1,true,true,true,true);
      
      break;
    case FM_UPDATE:
      break;
    case FM_KEYS:
      switch ( keycode ) {
        case 'c':
        case 'C':
          menumode = MM__START_TO_GAME;
          switchToState();
          return true;
        case 's':
        case 'S':
          menumode = MM__START_TO_SOLO;
          switchToState();
          return true;
        case 'h':
        case 'H':
          menumode = MM__START_TO_HOST;
          switchToState();
          return true;
        case 'j':
        case 'J':
          menumode = MM__START_TO_JOIN;
          switchToState();
          return true;
        case 'p':
        case 'P':
          menumode = MM__START_TO_PLAYER;
          switchToState();
          return true;
        case 'q':
        case 'Q':
          setRunning(false);
          return true;
        case 27:
          menumode = MM_MENU_IS_OFF;
          // there is no need to switchToState()
          return true;
      }
      break;
    case FM_INIT:
      break;
  }
  
  return false;
}



bool handle__START_TO_GAME(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_START_text(mainFade*mainFade*mainFade,true,true,true,true);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(mainFade <= 0.0){
        // dont' need to call the switch fcb for this state
        menumode = MM_MENU_IS_OFF;
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = (float)OVERLAY_X / 2;
      goalRightEdge = (float)OVERLAY_X / 2;
      goalTopEdge = (float)OVERLAY_Y / 2;
      goalBottomEdge = (float)OVERLAY_Y / 2;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.1;
      break;
  }
  
  return false;
}



bool handle_SOLO(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_SOLO_text(1.0,1.0);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade <= 0.0){
        // this should pretty much start a new game
        _netStatus = GAME_SOLO;
        resetForNewGame();
        menumode = MM__SOLO_TO_GAME;
        switchToState();
      }    
      break;
    case FM_KEYS:
      switch ( keycode ) {
        case 'c':
        case 'C':
        case 27:   // escape
          menumode = MM__SOLO_TO_START;
          switchToState();
          return true;
      }
      break;
    case FM_INIT:
      horizEdgeMoveRate = 0.0;
      vertEdgeMoveRate = 0.0;
      titleMoveRateX = 0.0;
      titleMoveRateY = 0.0;
      scndFade = 1.0;
      scndFadeRate = 0.01;
      break;
  }
  
  return false;
}


bool handle__START_TO_SOLO(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_SOLO_text(scndFade,localFadeB);
      
      // standardizes the drawing to reduce code size
      handle_START_text(localFadeA,false,true,true,true);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade >= 1.0){
        menumode = MM_SOLO;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = 300;
      goalRightEdge = 724;
      goalTopEdge = 498;
      goalBottomEdge = 270;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = 423;
      titleY = 350-37.5;
      goalTitleX = 423;
      goalTitleY = 440;
      titleMoveRateX = (goalTitleX - titleX) / 10;
      titleMoveRateY = (goalTitleY - titleY) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.0;
      scndFade = 0.0;
      scndFadeRate = -0.1;
      break;
  }
  
  return false;
}


bool handle__SOLO_TO_START(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_SOLO_text(scndFade,localFadeB);
      
      // standardizes the drawing to reduce code size
      handle_START_text(localFadeA,false,true,true,true);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade <= 0.0){
        menumode = MM_START;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = 100;
      goalRightEdge = 924;
      goalTopEdge = 668;
      goalBottomEdge = 100;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = 423;
      titleY = 440;
      goalTitleX = 423;
      goalTitleY = 350-37.5;
      titleMoveRateX = (goalTitleX - titleX) / 10;
      titleMoveRateY = (goalTitleY - titleY) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.0;
      scndFade = 1.0;
      scndFadeRate = 0.1;
      break;
  }
  
  return false;
}



bool handle__SOLO_TO_GAME(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_SOLO_text(1.0,mainFade * mainFade * mainFade);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(mainFade <= 0.0){
        // dont' need to call the switch fcb for this state
        menumode = MM_MENU_IS_OFF;
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = (float)OVERLAY_X / 2;
      goalRightEdge = (float)OVERLAY_X / 2;
      goalTopEdge = (float)OVERLAY_Y / 2;
      goalBottomEdge = (float)OVERLAY_Y / 2;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.1;
      break;
  }
  
  return false;
}



bool handle_HOST(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_HOST_text(1.0,1.0,1.0);
      
      break;
    case FM_UPDATE:
      
      // this looks for a connection but is not gaurenteed to do anything
      if(hostListen()){
        // proceed to the next menu
        menumode = MM__HOST_TO_SYNCRO;
        switchToState();
      }
      
      break;
    case FM_KEYS:
      switch ( keycode ) {
        case 'c':
        case 'C':
        case 27:   // escape
          menumode = MM__HOST_TO_START;
          switchToState();
          return true;
      }
      break;
    case FM_INIT:
      break;
  }
  
  return false;
}


bool handle__START_TO_HOST(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_HOST_text(scndFade,localFadeB,1.0);
      
      // standardizes the drawing to reduce code size
      handle_START_text(localFadeA,true,false,true,true);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade >= 1.0){
        menumode = MM_HOST;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = 300;
      goalRightEdge = 724;
      goalTopEdge = 498;
      goalBottomEdge = 270;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = 423+150;
      titleY = 350+37.5;
      goalTitleX = 423;
      goalTitleY = 440;
      titleMoveRateX = (goalTitleX - titleX) / 10;
      titleMoveRateY = (goalTitleY - titleY) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.0;
      scndFade = 0.0;
      scndFadeRate = -0.1;
      break;
  }
  
  return false;
}


bool handle__HOST_TO_START(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_HOST_text(scndFade,localFadeB,1.0);
      
      // standardizes the drawing to reduce code size
      handle_START_text(localFadeA,true,false,true,true);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade <= 0.0){
        menumode = MM_START;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = 100;
      goalRightEdge = 924;
      goalTopEdge = 668;
      goalBottomEdge = 100;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = 423;
      titleY = 440;
      goalTitleX = 423+150;
      goalTitleY = 350+37.5;
      titleMoveRateX = (goalTitleX - titleX) / 10;
      titleMoveRateY = (goalTitleY - titleY) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.0;
      scndFade = 1.0;
      scndFadeRate = 0.1;
      break;
  }
  
  return false;
}


bool handle_JOIN(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_JOIN_text(1.0,1.0,scndFade,1.0);
      
      break;
    case FM_UPDATE:
      // scndFade is being used as cursor flash here
      scndFade = .5 + .5 * sin((float)_timestamp / 159.15);
      
      // flash timer ... we flash the IP if it gets rejected
      if(errrFlashing){
        if(_timestamp - errStart > 1000){
          errrFlashing = false;
        }
      }
      break;
    case FM_KEYS:
      switch ( keycode ) {
        case 'c':
        case 'C':
        case 27:   // escape
          menumode = MM__JOIN_TO_START;
          switchToState();
          return true;
        case 13:   // return
        case 'g':
        case 'G':
          if( verifyIP(serverIP) && clientConnect(serverIP) ){
            menumode = MM__JOIN_TO_SYNCRO;
            switchToState();
            return true;
          }else{
            errrFlashing = true;
            errStart = _timestamp;
            return true;
          }
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          // reject key if we are in the first digit of one of the tripples
          if(selected % 4 == 0) break;
        case '0':
        case '1':
        case '2':
          serverIP[selected] = (char)keycode;
          ++selected;
          if(selected % 4 == 3)
            ++selected;
          if(selected > 14)
            selected = 0;
          return true;
        case 276:   // left arrow
        case 273:   // up arrow
          --selected;
          if(selected % 4 == 3)
            --selected;
          if(selected < 0)
            selected = 14;
          return true;
        case 275:   // right arrow
        case 274:   // down arrow
          ++selected;
          if(selected % 4 == 3)
            ++selected;
          if(selected > 14)
            selected = 0;
          return true;
      }
      break;
    case FM_INIT:
      errrFlashing = false;
      break;
  }
  
  return false;
}



bool handle__START_TO_JOIN(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_JOIN_text(scndFade,localFadeB,1.0,1.0);
      
      // standardizes the drawing to reduce code size
      handle_START_text(localFadeA,true,true,false,true);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade >= 1.0){
        menumode = MM_JOIN;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      // must initialize our default server IP address
      selected = 0;
      if(!serverIP){
        serverIP = new char[16];
        for(int i=0;i<15;++i){
          if(i % 4 != 3)
            serverIP[i] = 'x';
          else
            serverIP[i] = '.';
        }
        serverIP[15] = 0;
      }
      
      goalLeftEdge = 350;
      goalRightEdge = 674;
      goalTopEdge = 498;
      goalBottomEdge = 270;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = 426-150;
      titleY = 350+37.5;
      goalTitleX = 426;
      goalTitleY = 440;
      titleMoveRateX = (goalTitleX - titleX) / 10;
      titleMoveRateY = (goalTitleY - titleY) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.0;
      scndFade = 0.0;
      scndFadeRate = -0.1;
      break;
  }
  
  return false;
}



bool handle__JOIN_TO_START(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_JOIN_text(scndFade,localFadeB,1.0,1.0);
      
      // standardizes the drawing to reduce code size
      handle_START_text(localFadeA,true,true,false,true);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade <= 0.0){
        menumode = MM_START;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = 100;
      goalRightEdge = 924;
      goalTopEdge = 668;
      goalBottomEdge = 100;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = 426;
      titleY = 440;
      goalTitleX = 426-150;
      goalTitleY = 350+37.5;
      titleMoveRateX = (goalTitleX - titleX) / 10;
      titleMoveRateY = (goalTitleY - titleY) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.0;
      scndFade = 1.0;
      scndFadeRate = 0.1;
      break;
  }
  
  return false;
}



bool handle_PLAYER(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_PLAYER_text(1.0,1.0);
      
      break;
    case FM_UPDATE:
      break;
    case FM_KEYS:
      switch ( keycode ) {
        case 'd':
        case 'D':
        case 27:   // escape
          menumode = MM__PLAYER_TO_START;
          switchToState();
          return true;
      }
      break;
    case FM_INIT:
      break;
  }
  
  return false;
}



bool handle__START_TO_PLAYER(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_PLAYER_text(scndFade,localFadeB);
      
      // standardizes the drawing to reduce code size
      handle_START_text(localFadeA,true,true,true,false);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade >= 1.0){
        menumode = MM_PLAYER;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = 200;
      goalRightEdge = 824;
      goalTopEdge = 588;
      goalBottomEdge = 180;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = 463;
      titleY = 200;
      goalTitleX = 463;
      goalTitleY = 530;
      titleMoveRateX = (goalTitleX - titleX) / 10;
      titleMoveRateY = (goalTitleY - titleY) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.0;
      scndFade = 0.0;
      scndFadeRate = -0.1;
      break;
  }
  
  return false;
}



bool handle__PLAYER_TO_START(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_PLAYER_text(scndFade,localFadeB);
      
      // standardizes the drawing to reduce code size
      handle_START_text(localFadeA,true,true,true,false);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade <= 0.0){
        menumode = MM_START;
        switchToState();
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      goalLeftEdge = 100;
      goalRightEdge = 924;
      goalTopEdge = 668;
      goalBottomEdge = 100;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      titleX = 463;
      titleY = 530;
      goalTitleX = 463;
      goalTitleY = 200;
      titleMoveRateX = (goalTitleX - titleX) / 10;
      titleMoveRateY = (goalTitleY - titleY) / 10;
      mainFade = 1.0;
      mainFadeRate = 0.0;
      scndFade = 1.0;
      scndFadeRate = 0.1;
      break;
  }
  
  return false;
}



/*
       would be nice to have a paused game engine for this
       no fixed update rate
       each update we send one object, then return (for draw updates etc)
       
       "handshake"
       make _otherPlayerShip
       send package and send _playerShip
       send package and send _otherPlayerShip
       loop where I package and send each asteriod
       "handshake"
*/
bool handle_SYNCRO(functionmodetype mode, int keycode){
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      // standardizes the drawing to reduce code size
      handle_SYNCRO_text(1.0);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      // plan:
      // 1> get the starting handshake done
      // 2> start transferring objects
      // 3> get the ending handshake done
      
      switch ( oDone ){
        case -2:  // working on starting handshake
          if(doGreetings()){
            // yay, it worked so lets get on with it
            oDone = 0;
            errStart = 0;
          }
          
          ++errStart;
          if(errStart > 500){
            // eek, opening handshake failed (it timed out)
            menumode = MM__SYNCRO_TO_ERROR;
            switchToState();
          }
          
          break;
        case -1:  // working on ending handshake
          if(doEndGreetings()){
            // yay, lets start the game
            menumode = MM__SYNCRO_TO_GAME;
            switchToState();
          }
          
          ++errStart;
          if(errStart > 500){
            // eek, ending handshake failed (it timed out)
            menumode = MM__SYNCRO_TO_ERROR;
            switchToState();
          }
          
          break;
        default:  // working on object transfers
          if(doObjectTransfer(oPtr)){
            ++oDone;
            if(oPtr) oPtr = oPtr->getNext();
            errStart = 0;
            
            // go to endHandshake
            if(oDone >= _numObj) oDone = -1;
          }
          
          ++errStart;
          if(errStart > 500){
            // eek, object transfer failed (it timed out)
            menumode = MM__SYNCRO_TO_ERROR;
            switchToState();
          }
          
          break;
      }
      
      break;
    case FM_KEYS:
      switch ( keycode ) {
        case 'c':
        case 'C':
        case 27:   // escape
          menumode = MM__SYNCRO_TO_START;
          switchToState();
          
          // reset game to work as a solo game
          _netStatus = GAME_SOLO;
          resetForNewGame();
          setPaused(false);
          
          return true;
      }
      break;
    case FM_INIT:
      
      if(_netStatus == GAME_HOST){
        // starting network and game setup for new game
        resetForNewGame();
        setPaused(true);
        oPtr = _objects;
      }else{
        oPtr = NULL;
      }
      
      // this is how we time out on things
      errStart = 0;
      
      break;
  }
  
  return false;
}



bool handle__HOST_TO_SYNCRO(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_SYNCRO_text(localFadeB);
      
      // standardizes the drawing to reduce code size
      handle_HOST_text(1.0,localFadeA,localFadeA);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade >= 1.0){
        menumode = MM_SYNCRO;
        switchToState();
        
        _netStatus = GAME_HOST;
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      /*
      goalLeftEdge = 300;
      goalRightEdge = 724;
      goalTopEdge = 498;
      goalBottomEdge = 270;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      */
      
      horizEdgeMoveRate = 0.0;
      vertEdgeMoveRate = 0.0;
      titleMoveRateX = 0.0;
      titleMoveRateY = 0.0;
      
      scndFade = 0.0;
      scndFadeRate = -0.1;
      
      oDone = -2;
      break;
  }
  
  return false;
}



bool handle__JOIN_TO_SYNCRO(functionmodetype mode, int keycode){
  float localFadeA, localFadeB;
  
  switch (mode) {
    case FM_DRAW:
      drawMenuBackground();
      
      localFadeA = 1 - scndFade;
      localFadeA = localFadeA * localFadeA * localFadeA;
      localFadeB = scndFade * scndFade * scndFade;
      
      // standardizes the drawing to reduce code size
      handle_SYNCRO_text(localFadeB);
      
      // standardizes the drawing to reduce code size
      handle_JOIN_text(1.0,localFadeA,1.0,localFadeA);
      
      break;
    case FM_UPDATE:
      basicMenuUpdate();
      
      if(scndFade >= 1.0){
        menumode = MM_SYNCRO;
        switchToState();
        
        _netStatus = GAME_JOIN;
      }
      break;
    case FM_KEYS:
      break;
    case FM_INIT:
      /*
      goalLeftEdge = 300;
      goalRightEdge = 724;
      goalTopEdge = 498;
      goalBottomEdge = 270;
      horizEdgeMoveRate = (goalRightEdge - rightEdge) / 10;
      vertEdgeMoveRate = (goalTopEdge - topEdge) / 10;
      */
      
      horizEdgeMoveRate = 0.0;
      vertEdgeMoveRate = 0.0;
      titleMoveRateX = 0.0;
      titleMoveRateY = 0.0;
      
      scndFade = 0.0;
      scndFadeRate = -0.1;
      
      oDone = -2;
      break;
  }
  
  return false;
}



bool handle__SYNCRO_TO_START(functionmodetype mode, int keycode){
  return false;
}



bool handle__SYNCRO_TO_GAME(functionmodetype mode, int keycode){
  return false;
}



bool handle_ERROR(functionmodetype mode, int keycode){
  return false;
}



bool handle__SYNCRO_TO_ERROR(functionmodetype mode, int keycode){
  return false;
}



bool handle__ERROR_TO_START(functionmodetype mode, int keycode){
  return false;
}



void switchToState(){
  (*menuFuncs[menumode])(FM_INIT,0);
}



void initMenu(){
  //-------------------------------------------------------------------------
  // specific fill of menuFuncs array
  
  menuFuncs[MM__PRE_START] = handle__PRE_START;
  menuFuncs[MM_START] = handle_START;
  menuFuncs[MM__START_TO_GAME] = handle__START_TO_GAME;
  menuFuncs[MM_SOLO] = handle_SOLO;
  menuFuncs[MM__START_TO_SOLO] = handle__START_TO_SOLO;
  menuFuncs[MM__SOLO_TO_START] = handle__SOLO_TO_START;
  menuFuncs[MM__SOLO_TO_GAME] = handle__SOLO_TO_GAME;
  menuFuncs[MM_HOST] = handle_HOST;
  menuFuncs[MM__START_TO_HOST] = handle__START_TO_HOST;
  menuFuncs[MM__HOST_TO_START] = handle__HOST_TO_START;
  menuFuncs[MM_JOIN] = handle_JOIN;
  menuFuncs[MM__START_TO_JOIN] = handle__START_TO_JOIN;
  menuFuncs[MM__JOIN_TO_START] = handle__JOIN_TO_START;
  menuFuncs[MM_PLAYER] = handle_PLAYER;
  menuFuncs[MM__START_TO_PLAYER] = handle__START_TO_PLAYER;
  menuFuncs[MM__PLAYER_TO_START] = handle__PLAYER_TO_START;
  menuFuncs[MM_SYNCRO] = handle_SYNCRO;
  menuFuncs[MM__HOST_TO_SYNCRO] = handle__HOST_TO_SYNCRO;
  menuFuncs[MM__JOIN_TO_SYNCRO] = handle__JOIN_TO_SYNCRO;
  menuFuncs[MM__SYNCRO_TO_START] = handle__SYNCRO_TO_START;
  menuFuncs[MM__SYNCRO_TO_GAME] = handle__SYNCRO_TO_GAME;
  menuFuncs[MM_ERROR] = handle_ERROR;
  menuFuncs[MM__SYNCRO_TO_ERROR] = handle__SYNCRO_TO_ERROR;
  menuFuncs[MM__ERROR_TO_START] = handle__ERROR_TO_START;
  
  //-------------------------------------------------------------------------
  // set up the "normal" varialbles
  
  menumode = MM__PRE_START;
  switchToState();
  
  readServersList();
  
  serverIP = NULL;
}



void drawMenu(){
  if(menumode == MM_MENU_IS_OFF) return;
  
  setOverlayMode();
  
  (*menuFuncs[menumode])(FM_DRAW,0);
  
  setNormalMode();
}



void updateMenu(){
  if(menumode == MM_MENU_IS_OFF) return;
  
  // yeah passing a constant float does seem a bit stupid, I'll make things nice some day
  (*menuFuncs[menumode])(FM_UPDATE,0);
}



bool keyToMenu(int keycode){
  if(menumode == MM_MENU_IS_OFF){
    if(keycode == 27){
      // they pressed escape and the menu is off, so turn menu on
      menumode = MM__PRE_START;
      switchToState();
      return true;
    }else{
      return false;
    }
  }
  return (*menuFuncs[menumode])(FM_KEYS,keycode);
}
