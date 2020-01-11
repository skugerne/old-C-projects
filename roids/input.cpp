#include "main.hpp"



#ifdef DEBUG_KEYBOARD
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
#endif



void handleKeyDown(SDL_KeyboardEvent *key){
  int keycode = key->keysym.sym;
  
  #ifdef DEBUG_KEYBOARD
  fprintf(stderr,"Key %d down: \" %s \" .\n", keycode,SDL_GetKeyName( (SDLKey)keycode ) );
  PrintModifiers( key->keysym.mod );
  #endif
  
  // if the menu is operating, (most) keys are intercepted here
  if(keyToMenu(keycode))
    return;
  
  for(Uint i=0;i<NUM_KEYS;++i)
    if(_keys[i] == 0){
      _keys[i] = keycode;
      i = NUM_KEYS + 1;
    }
  
  #ifdef DEBUG_KEYBOARD
  for(int i=0;i<NUM_KEYS;++i)
    fprintf(stderr,"[%d]", _keys[i]);
  fprintf(stderr,"\n");
  #endif
  
  // lets check for some global key presses
  switch ( keycode ) {
    case 282:  // F1 ... toggle full screen mode
      _videoFlags ^= SDL_FULLSCREEN;
      // I think that this method of handling screen size is lame
      switch ( _screen->w ) {
        case 640:
          initDraw( RES_640x480 );
          break;
        case 800:
          initDraw( RES_800x600 );
          break;
        default:
          initDraw( RES_1024x768 );
          break;
      }
      break;
    case 283: // F2 ... toggle resolutions
      // I think that this method of handling screen size is lame
      switch ( _screen->w ) {
        case 640:
          initDraw( RES_800x600 );
          break;
        case 800:
          initDraw( RES_1024x768 );
          break;
        case 1024:
          initDraw( RES_640x480 );
          break;
        default:
          initDraw( RES_1024x768 );
          break;
      }
      break;
    case 284: // F3 ... toggle IP enter mode
      if(_player) _player->addShake(1000.0);
      break;
    case 285:                     // ** F4 **
      if(_viewMode == VIEW_WORLD_AXIS_ALIGNED){
        _viewMode = VIEW_PLAYER_AXIS_ALIGNED;
      }else if(_viewMode == VIEW_PLAYER_AXIS_ALIGNED){
        _viewMode = VIEW_SIDESCROLL_ALIGNED;
      }else if(_viewMode == VIEW_SIDESCROLL_ALIGNED){
        _viewMode = VIEW_PLAYER_MOTION_ALIGNED;
      }else{
        _viewMode = VIEW_WORLD_AXIS_ALIGNED;
      }
      break;
    case 286:                     // ** F5 **
      _option1 = !_option1;
      break;
    case 287:                     // ** F6 **
      _option2 = !_option2;
      break;
    case 288:                     // ** F7 **
      _option3 = !_option3;
      break;
    case 289:                     // ** F8 **
      _option4 = !_option4;
      break;
    case 290:                     // ** F9 **
      break;
  }
  
  // check for the control-escape combo ... if we find it, quit
  bool quit = false;
  for(Uint i=0;i<NUM_KEYS;++i)
    if((!quit && _keys[i] == 306) || (!quit && _keys[i] == 27)) quit = true;
    else if((quit && _keys[i] == 306) || (quit && _keys[i] == 27)){
      setRunning(false);
    }
}



void handleKeyUp(SDL_KeyboardEvent *key){
  int keycode = key->keysym.sym;
  
  #ifdef DEBUG_KEYBOARD
  fprintf(stderr,"Key %d up: \" %s \" .\n", keycode,SDL_GetKeyName( (SDLKey)keycode ) );
  PrintModifiers( key->keysym.mod );
  #endif
  
  for(int i=0;i<NUM_KEYS;++i)
    if(_keys[i] == keycode)
      _keys[i] = 0;
  
  #ifdef DEBUG_KEYBOARD
  for(int i=0;i<NUM_KEYS;++i)
    fprintf(stderr,"[%d]", _keys[i]);
  fprintf(stderr,"\n");
  #endif
}
