#include "main.h"
#include "font.h"
#include <sys/stat.h>



fontlist_t *fontPile;

// remembering settings for font rendering
// these are changed as need, much as with openGL settings
unsigned curSize;
char curName[100];
float curR, curG, curB;
float curX, curY, curA;
textjusty_t curJustX,curJustY;
glyphlist_t *curGlyphs;



// set pointers to NULL, etc
void initText(){
  fontPile = NULL;
  
  curName[0] = 0;
  curGlyphs = NULL;
  curX = 0.0;
  curY = 0.0;
  curA = 0.0;
  curJustX = TEXT_RIGHT;
  curJustY = TEXT_TOP;
  
  loadFont("fonts/liquidn.ttf","lcd");
  loadFont("fonts/minima1.ttf","min");
  loadFont("fonts/nova.ttf","nova");
}



int isFile(char *file){
  struct stat fileStats;
  int result;
  
  if( !file ){
    fprintf(stderr,"ERROR - Passed NULL file to isFile.\n");
    return 0;
  }
  
  result = stat(file,&fileStats);
  if( result != -1 && S_ISREG(fileStats.st_mode) ){
    return 1;
  }
  return 0;
}



// load a font at the given path, associate with the given handle
// creates a fontlist_t object for each font
void loadFont(char *path, char *name){
  if( !isFile(path) ){
    fprintf(stderr,"WARNG - font '%s' not loaded.\n",path);
    return;
  }
  
  fontlist_t *newfont = new fontlist_t;
  if( !newfont ){
    fprintf(stderr,"ERROR - failed to allocate fontlist.\n");
    exit(-1);
  }
  
  // copy the string, memory allocated with malloc
  // hmmm, is this a problem with C++?
  newfont->name = strdup(name);
  newfont->path = strdup(path);
  
  newfont->glyphs = NULL;
  newfont->next = fontPile;
  fontPile = newfont;
}



// set text color, font and size
// will create new glyphlist_t things when the size is new
void setTextProps(float r, float g, float b, char *name, unsigned size){
  curR = r;
  curG = g;
  curB = b;
  strncpy(curName,name,100);
  curName[99] = 0;
  curSize = size;
  
  fontlist_t *fPtr = fontPile;
  while( fPtr && strncmp(fPtr->name,name,99) != 0 )
    fPtr = fPtr->next;
  
  if( !fPtr ){
    fprintf(stderr,"ERROR - Unable to find font '%s'.\n",name);
    exit(-1);
  }
  
  // so if we found the font, now look for the size
  glyphlist_t *gPtr = fPtr->glyphs;
  while( gPtr && gPtr->size != size )
    gPtr = gPtr->next;
  
  // if we didn't find the size, make a size entry
  if( !gPtr ){
    #ifdef TTF_TEST_DEBUG
    fprintf(stderr,"About to create a new glyphlist.\n");
    fprintf(stderr,"Trying to load '%s' for '%s'.\n",fPtr->path,name);
    #endif
    
    gPtr = new glyphlist_t;
    if( !gPtr ){
      fprintf(stderr,"ERROR - failed to allocate glyphlist.\n");
      exit(-1);
    }
    
    if( !(gPtr->font = TTF_OpenFont(fPtr->path,size)) ){
      fprintf(stderr,"ERROR - failed to load font: %s", TTF_GetError());
      exit(-1);
    }
  
    gPtr->size = size;
    gPtr->font_h = 0;
    gPtr->font_h_diff = 0;
    for(int i=0;i<256;++i)
      gPtr->textures[i] = 0;
    
    gPtr->next = fPtr->glyphs;
    fPtr->glyphs = gPtr;
    
    #ifdef TTF_TEST_DEBUG
    fprintf(stderr,"Done creating glyphlist.\n");
    #endif
  }
  
  curGlyphs = gPtr;
}



// place the cursor at the specified coords
void setTextStart(float x, float y){
  curX = x;
  curY = y;
  curA = 0.0;
}



// place the cursor at the specified coords and prepare to print rotated text
void setTextStart(float x, float y, float angle){
  curX = x;
  curY = y;
  curA = angle;
}


// place the cursor at the specified coords with specified alignment
void setTextJust(textjusty_t justx, textjusty_t justy){
  curJustX = justx;
  curJustY = justy;
}



// adjust the location of the cursor relative to where it is
void shiftTextStart(float x, float y){
  curX += x;
  curY += y;
}



int makePow2(int x){
  int i=1;
  while( i<x ) i *= 2;
  return i;
}



// how far ahead should the next character be printed?
void storeAdvance(unsigned char index){
  int minx, maxx, miny, maxy;
  TTF_GlyphMetrics(
    curGlyphs->font,index, &minx, &maxx, &miny, &maxy, &curGlyphs->glyph_advance[index]
  );
}



void createGlyph(unsigned char index){
  SDL_Color color;
  SDL_Surface *initial, *intermediary;
  
  // color to render in (which is always white because openGL will change it)
  color.r = 255;
  color.g = 255;
  color.b = 255;
  
  // put our character into a very short string
  char str[2];
  str[0] = index;
  str[1] = 0;
  
  // obtain our glyph on a fitted SDL surface
  initial = TTF_RenderText_Blended(curGlyphs->font, str, color);
  
  #ifdef TTF_TEST_DEBUG
  fprintf(stderr,"Got %d by %d SDL surface.\n",initial->w,initial->h);
  #endif
  
  // store properties of this surface
  storeAdvance(index);
  curGlyphs->tex_w[index] = makePow2(initial->w);
  curGlyphs->tex_h[index] = makePow2(initial->h);
  if( curGlyphs->font_h == 0 ){
    curGlyphs->font_h = TTF_FontHeight(curGlyphs->font);
    curGlyphs->font_h_diff = curGlyphs->tex_h[index] - curGlyphs->font_h;
  }
  
  // copy the fitted data into a power-of-2 sized one
  intermediary = SDL_CreateRGBSurface(
    SDL_SWSURFACE, curGlyphs->tex_w[index],
    curGlyphs->tex_h[index], 32, 
    0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
  );
  SDL_BlitSurface(initial, 0, intermediary, 0);
  
  // generate an openGL texture of the power-of-2 size
  glGenTextures(1, &curGlyphs->textures[index]);
  glBindTexture(GL_TEXTURE_2D, curGlyphs->textures[index]);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA, curGlyphs->tex_w[index], 
    curGlyphs->tex_h[index], 0, GL_RGBA, GL_UNSIGNED_BYTE,
    intermediary->pixels
  );
  
  // set some mysterious parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  // clean up our temporary stuff
  SDL_FreeSurface(initial);
  SDL_FreeSurface(intermediary);
}



void drawGlyph(unsigned char index){
  glEnable(GL_TEXTURE_2D);
  
  glBindTexture(GL_TEXTURE_2D, curGlyphs->textures[index]);
  glColor4f(curR, curG, curB, 0.3);
  
  glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); 
    glVertex2f(curX, curY);
    glTexCoord2f(1.0, 1.0); 
    glVertex2f(curX + curGlyphs->tex_w[index], curY);
    glTexCoord2f(1.0, 0.0); 
    glVertex2f(curX + curGlyphs->tex_w[index], curY + curGlyphs->tex_h[index]);
    glTexCoord2f(0.0, 0.0); 
    glVertex2f(curX, curY + curGlyphs->tex_h[index]);
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  
  curX += (float)curGlyphs->glyph_advance[index];
}



// determine how wide the given string will be when printed
int textWidth(char *str){
  if( !str ){
    fprintf(stderr,"WARNG - Passed NULL str to textWidth.\n");
    return 0;
  }
  if( !curName[0] ){
    fprintf(stderr,"WARNG - Attempt to measure unconfigured text.\n");
    return 0;
  }
  
  int result = 0;
  int index = 0;
  unsigned char *uStr = (unsigned char *)str;
  while( uStr[index] ){
    result += curGlyphs->glyph_advance[uStr[index]];
    ++index;
  }
  
  return result;
}



// determine how tall the tallest string would be when printed
int textHieght(){
  if( !curName[0] ){
    fprintf(stderr,"WARNG - Attempt to measure unconfigured text.\n");
    return 0;
  }
  return curGlyphs->font_h;
}



// write a string of text to the screen, creating glyphs as needed
void renderText(char *str){
  if( !str ){
    fprintf(stderr,"WARNG - Passed NULL str to renderText.\n");
    return;
  }
  if( !curName[0] ){
    fprintf(stderr,"WARNG - Attempt to render unconfigured text.\n");
    return;
  }
  
  int index;
  unsigned char *uStr = (unsigned char *)str;
  
  // create glyphs as needed
  index = 0;
  while( uStr[index] ){
    if( !curGlyphs->textures[uStr[index]] )
      createGlyph(uStr[index]);
    ++index;
  }
  
  if( curA != 0.0 ){
    glPushMatrix();
    glTranslatef(curX,curY,0.0);
    glRotatef(curA,0.0,0.0,1.0);
    
    curX = 0.0;
    curY = 0.0;
  }
  
  // apply any needed x offset
  if( curJustX == TEXT_CENTER )
    curX -= (float)(textWidth(str)) / 2.0;
  else if( curJustX == TEXT_RIGHT )
    curX -= (float)textWidth(str);
  
  // apply any needed y offset
  if( curJustY == TEXT_CENTER )
    curY -= (float)curGlyphs->font_h / 2.0;
  else if( curJustY == TEXT_BOTTOM )
    curY -= (float)curGlyphs->font_h;
  
  // compensate for the difference in openGL texture size and useful size
  curY -= (float)curGlyphs->font_h_diff;
  
  // draw glyphs
  index = 0;
  while( uStr[index] ){
    drawGlyph(uStr[index]);
    ++index;
  }
  
  if( curA != 0.0 )
    glPopMatrix();
}


