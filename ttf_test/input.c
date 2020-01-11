#include "main.h"
#include "font.h"



void PrintModifiers( SDLMod mod ){
  if( mod == KMOD_NONE ){
    return;
  }
  
  if( mod & KMOD_NUM ) fprintf(stderr,"NUMLOCK ");
  if( mod & KMOD_CAPS ) fprintf(stderr,"CAPSLOCK ");
  if( mod & KMOD_LCTRL ) fprintf(stderr,"LCTRL ");
  if( mod & KMOD_RCTRL ) fprintf(stderr,"RCTRL ");
  if( mod & KMOD_RSHIFT ) fprintf(stderr,"RSHIFT ");
  if( mod & KMOD_LSHIFT ) fprintf(stderr,"LSHIFT ");
  if( mod & KMOD_RALT ) fprintf(stderr,"RALT ");
  if( mod & KMOD_LALT ) fprintf(stderr,"LALT ");
  if( mod & KMOD_CTRL ) fprintf(stderr,"CTRL ");
  if( mod & KMOD_SHIFT ) fprintf(stderr,"SHIFT ");
  if( mod & KMOD_ALT ) fprintf(stderr,"ALT ");
  fprintf(stderr,"\n");
}



void handleKeyDown(SDL_KeyboardEvent *key){
  int keycode = key->keysym.sym;
  
  switch ( keycode ) {
    case 282:       // F1
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      break;
    case 283:       // F2
      glBlendFunc(GL_ONE, GL_ONE);
      break;
    case 284:       // F3
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
    #ifdef FONT_DEBUG
    case 285:       // F4
      g_borders = !g_borders;
      break;
    case 286:       // F5
      g_stepUp = !g_stepUp;
      break;
    case 287:       // F6
      newFonts();
      break;
    case 288:       // F7
      if( g_textVert == TEXT_CENTER ) g_textVert = TEXT_BOTTOM;
      else if( g_textVert == TEXT_BOTTOM ) g_textVert = TEXT_TOP;
      else g_textVert = TEXT_CENTER;
      break;
    #endif
    case 27:        // esc
      setRunning(false);
      break;
  }
}



void handleKeyUp(SDL_KeyboardEvent *key){
  //int keycode = key->keysym.sym;
}
