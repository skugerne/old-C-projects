#include "main.hpp"



// used to store widths/hieghts of the font in normal and small mode
double smallH, smallV;
double mediumH, mediumV;

bool overlayMode;



// switch us into the overlay coordinate system
void setOverlayMode(){
  if(!overlayMode){
    overlayMode = true;
    
    glEnable(GL_TEXTURE_2D);
    
    // -----------------------------------------------------------------------
    // NOTE:  In order to work around an issue on a win2000 test machine, ...
    // ... I had to remove the glPushMatrix() here and replace the pop in ...
    // ... setNormalMode() with a stack-whiping alternative.  This is bad. ...
    // ... Perhaps some day it will be fixed.
    // SYMPTOMS: Drawing text whipes out the GL stack.
    // -----------------------------------------------------------------------
    
    #ifndef STACK_POP_BUG
    glPushMatrix();
    #endif
    
    glLoadIdentity();
    glOrtho( 0, OVERLAY_X, 0, OVERLAY_Y, -1, 1 );
  }else{
    #ifdef DEBUG
    fprintf(stderr,"WARNG - setOverlayMode() called when overlayMode == true.\n");
    #endif
  }
}



// switch us back to the normal system (I hope you're in overlay)
void setNormalMode(){
  if(overlayMode){
    overlayMode = false;
    
    glDisable(GL_TEXTURE_2D);
    
    // -----------------------------------------------------------------------
    // NOTE:  In order to work around an issue on a win2000 test machine, ...
    // ... I had to remove the glPushMatrix() in setOverlayMode() and ...
    // ... replace the pop here with a stack-whiping alternative.  This is ...
    // ... bad.  Perhaps some day it will be fixed.
    // SYMPTOMS: Drawing text whipes out the GL stack.
    // -----------------------------------------------------------------------
    
    #ifdef STACK_POP_BUG
    glLoadIdentity();
    glOrtho(-_virtualPosX,_virtualPosX,-_virtualPosY,_virtualPosY,-1,1);
    #else
    glPopMatrix();
    #endif
  }else{
    #ifdef DEBUG
    fprintf(stderr,"WARNG - setNormalMode() called when overlayMode == false.\n");
    #endif
  }
}



// string output ... string:  **NULL terminated**
void printString(fonttype font, char *chars, int len, bool italic){
  glListBase(-31 + ( 128 * italic ));
  glBindTexture(GL_TEXTURE_2D, _texture[0]);
  
  glCallLists(len, GL_BYTE, chars);
}



// string output ... string:  **NULL terminated**
// negativeOffset is in terms of the overlay coordinate system
void printStringWithOffset(fonttype font, bool italic, char *chars, 
  double x, double y, double negativeOffset
){
  // flag for properly undoing what things we do to get set up
  bool alreadyOverlaid;
  
  if(!overlayMode){
    alreadyOverlaid = false;
    
    // since we're not already in overlayMode, lets scale these coordinates ...
    // ... for overlay mode, assuming that the programmer has sent us ...
    // ... "big picture" coordinates   ...   (-virtPosX/Y to +virtPosX/Y)
    x = (x + _virtualPosX) * ( (double)OVERLAY_X / (double)( 2 * _virtualPosX ));
    y = (y + _virtualPosY) * ( (double)OVERLAY_Y / (double)( 2 * _virtualPosY ));
    
    setOverlayMode();
  }else{
    alreadyOverlaid = true;
    
    glPushMatrix();
  }
    
  glTranslated( x - negativeOffset, y, 0 );
  
  printString(font,chars,strlen(chars),italic);
  
  // return us to our previous state
  if(!alreadyOverlaid){
    setNormalMode();
  }else{
    glPopMatrix();
  }
}



void printStringToLeft(fonttype font, bool italic, char *chars, double x, double y){
  printStringWithOffset(font,italic,chars,x,y,stringXOverlay(font,strlen(chars))+10);
}



// string output ... string:  **NULL terminated**
void printStringToRight(fonttype font, bool italic, char *chars, double x, double y){
  printStringWithOffset(font,italic,chars,x,y,0.0);
}



// string output ... string:  **NULL terminated**
void printStringCentered(fonttype font, bool italic, char *chars, double x, double y){
  printStringWithOffset(font,italic,chars,x,y,stringXOverlay(font,strlen(chars))/2+5);
}



// output of a single character, using existing functions as much as possible
void printCharToRight(fonttype font, bool italic, char oneChar, double x, double y){
  char *chars = new char[2];
  chars[0] = oneChar;
  chars[1] = 0;
  printStringWithOffset(font,italic,chars,x,y,0.0);
}



// string info ... string:  **NULL terminated**
double stringXOverlay(fonttype font, int len){
  if( font == FONT_MEDIUM ){
    return mediumH * (double)len;
  } else {
    return smallH * (double)len;
  }
}



// string info ... string:  **NULL terminated**
double stringX(fonttype font, char *chars){
  #ifdef DEBUG
  if(chars == NULL){
    fprintf(stderr,"ERROR - chars is NULL at stringX.\n");
    return 0.0;
  }
  #endif
  
  return stringX(font, strlen(chars));
}



// string info
double stringX(fonttype font, int len){
  double val;
  
  if( font == FONT_MEDIUM ){
    val = mediumH * (double)len;
  } else {
    val = smallH * (double)len;
  }
  
  if(!overlayMode)
    val *= ( (double)OVERLAY_X / (double)( 2 * _virtualPosX ) );
    
  return val;
}



// string info ... its assummed all characters are the same hieght
double stringY(fonttype font){
  double val;
  
  if( font == FONT_MEDIUM ){
    val = mediumV;
  } else {
    val = smallV;
  }
  
  if(!overlayMode)
    val *= ( (double)OVERLAY_Y / (double)( 2 * _virtualPosY ) );
    
  return val;
}



// initialization ... loads and processes the font into GL display lists
// I had originally planned for more than one font, but one has proved ...
// ... flexable enough (along with some hard-code titles)
bool loadFontTexture(){
  // initialization thing
  overlayMode = false;
  
  // --------------------
  // *** kludge below ***
  // --------------------
  smallH = 10;
  smallV = 16;
  mediumH = 10;
  mediumV = 16;
  // --------------------
  // *** kludge above ***
  // --------------------
  
  SDL_Surface *fontImage;
  /*
  int base = glGenLists(256);
  if(base != 1){
    fprintf(stderr,"WARNG - Font textures starting at %d.\n",base);
    fprintf(stderr,"ERROR - Font textures must have displists starting at 1.\n");
    return false;
  }
  */
  // read in the textures
  if( (fontImage = SDL_LoadBMP( "textures/font.bmp" )) ){
    glGenTextures( NUM_TEXTURES, &_texture[0] );
    
    #ifdef DEBUG
    fprintf(stderr,"Processing font characters:\n");
    #endif
    
    glBindTexture( GL_TEXTURE_2D, _texture[0] );
    
    // image width and hieght
    int wid = fontImage->w;
    int hig = fontImage->h;
    
    // check to make sure we have even size characters, in 16x16 grid
    if((wid % 16 != 0) || (hig % 16 != 0)){
      fprintf(stderr,"ERROR - bad dimention on font texture.\n");
      return false;
    }
    
    glTexImage2D( GL_TEXTURE_2D, 0, 3, wid, hig, 0, GL_RGB,
      GL_UNSIGNED_BYTE, fontImage->pixels
    );
     
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      
    // --------------------------------------------------------------
    // now we will convert this texture into 256 little display lists
      
    // x and y coordinates of characters
    double cx, cy;
    
    for(int loop=0;loop<256;++loop){
      // the bmp format is screwed up
    
      cx = 1 - ( double )( loop % 16 ) / 16.0;
      cy = 1 - ( double )( loop / 16 ) / 16.0;

      glNewList( ( 256 - loop ), GL_COMPILE );
        glBegin( GL_QUADS );
          // bottom left
          glTexCoord2f( cx - 0.0625, cy );
          glVertex2i( 0, 0 );
          
          // bottom right
          glTexCoord2f( cx, cy );
          glVertex2i( 16, 0 );
          
          // top right
          glTexCoord2f( cx, cy - 0.0625 );
          glVertex2i( 16, 16 );
          
          // top left
          glTexCoord2f( cx - 0.0625, cy - 0.0625);
          glVertex2i( 0, 16 );
        glEnd( );

        // move to left of char, so we're ready for next printing
        glTranslated( 10, 0, 0 );
      glEndList( );
    }
    
    // clear the memory
    if(fontImage) SDL_FreeSurface( fontImage );
    
    #ifdef DEBUG
    fprintf(stderr,"Font texture loaded.\n");
    #endif
    return true;
  }
  
  fprintf(stderr,"ERROR - Font textures were not loaded.  Files are suspect.\n");
  return false;
}



// load game textures (other than the font texture)
bool loadTextures(){
  // fontImage[0] should remain NULL
  SDL_Surface *fontImage[NUM_TEXTURES];

  if( (fontImage[1] = SDL_LoadBMP( "textures/close1.bmp" ))
    && (fontImage[2] = SDL_LoadBMP( "textures/close2.bmp" ))
    && (fontImage[3] = SDL_LoadBMP( "textures/sologame1.bmp" ))
    && (fontImage[4] = SDL_LoadBMP( "textures/sologame2.bmp" ))
    && (fontImage[5] = SDL_LoadBMP( "textures/hostgame1.bmp" ))
    && (fontImage[6] = SDL_LoadBMP( "textures/hostgame2.bmp" ))
    && (fontImage[7] = SDL_LoadBMP( "textures/joingame1.bmp" ))
    && (fontImage[8] = SDL_LoadBMP( "textures/joingame2.bmp" ))
    && (fontImage[9] = SDL_LoadBMP( "textures/player1.bmp" ))
    && (fontImage[10] = SDL_LoadBMP( "textures/player2.bmp" ))
    && (fontImage[11] = SDL_LoadBMP( "textures/quit1.bmp" ))
    && (fontImage[12] = SDL_LoadBMP( "textures/quit2.bmp" ))
    && (fontImage[13] = SDL_LoadBMP( "textures/opengl.bmp" ))
    && (fontImage[14] = SDL_LoadBMP( "textures/pattern.bmp" ))
    && (fontImage[15] = SDL_LoadBMP( "textures/sdl.bmp" ))
    && (fontImage[16] = SDL_LoadBMP( "textures/title_1.bmp" ))
    && (fontImage[17] = SDL_LoadBMP( "textures/title_2.bmp" ))
    && (fontImage[18] = SDL_LoadBMP( "textures/title_3.bmp" ))
    && (fontImage[19] = SDL_LoadBMP( "textures/connecting.bmp" ))
    && (fontImage[20] = SDL_LoadBMP( "textures/error.bmp" ))
  ){
    #ifdef DEBUG
    fprintf(stderr,"Processing textures: \n");
    #endif
    
    // note we start at 1, cause texture[0] is the special case font texture
    for(int i=1;i<NUM_TEXTURES;++i){
      glBindTexture( GL_TEXTURE_2D, _texture[i] );

      // image width and hieght
      int wid = fontImage[i]->w;
      int hig = fontImage[i]->h;

      glTexImage2D( GL_TEXTURE_2D, 0, 3, wid, hig, 0, GL_RGB,
        GL_UNSIGNED_BYTE, fontImage[i]->pixels
      );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

      // --------------------------------------------------------------
      // now we will convert this texture into display lists

      #ifdef DEBUG
      fprintf(stderr," %d x %d\n",wid,hig);
      #endif

      glNewList( 256 + i , GL_COMPILE );
        glBindTexture( GL_TEXTURE_2D, _texture[i] );
        glBegin( GL_QUADS );
          // bottom left
          glTexCoord2f( 0, 1 );
          glVertex2i( 0, 0 );

          // bottom right
          glTexCoord2f( 1, 1 );
          glVertex2i( wid, 0 );

          // top right
          glTexCoord2f( 1, 0 );
          glVertex2i( wid, hig );

          // top left
          glTexCoord2f( 0, 0 );
          glVertex2i( 0, hig );
        glEnd( );
      glEndList( );
    }

    // clear the memory
    for(int i=1;i<NUM_TEXTURES;++i)
      if(fontImage[i]) SDL_FreeSurface( fontImage[i] );

    #ifdef DEBUG
    fprintf(stderr,"Loaded.\n");
    #endif
    return true;
  }

  fprintf(stderr,"ERROR - Textures were not loaded.  Files are suspect.\n");
  return false;
}

