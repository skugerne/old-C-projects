#include "main.hpp"



Uint lastMSec;
Uint lastShipTransMSec;

// alien creation timers
Uint lastAF_MSec, lastAR_MSec, lastAW_MSec, lastAL_MSec;



void initUpdate(){
  #ifdef DEBUG_UPDATE
  fprintf(stderr,"Initializing update.\n");
  #endif
  
  lastMSec = SDL_GetTicks();
  lastShipTransMSec = 0;
  
  // alien creation timers
  lastAF_MSec = 0;
  lastAR_MSec = 0;
  lastAW_MSec = 0;
  lastAL_MSec = 0;
  
  // timestamp is incremented once per loop
  _timestamp = 1;
  _secCountStart = 0;
  
  // used primarily for the _fps and _ups count
  _frames = 0;
  _updates = 0;
  
  #ifdef DEBUG_UPDATE
  fprintf(stderr,"Zero out the sectors.\n");
  #endif
  for(int i=0;i<NUM_SECTORS_PER_SIDE;++i){
    for(int j=0;j<NUM_SECTORS_PER_SIDE;++j){
      
      // rather than set all 1,000,000+ _sector elements to NULL all the time
      //   we just use the timestamp to know when elements are out of date
      _sectors[i][j].first = NULL;
      _sectors[i][j].timestamp = 0;
      
      double xDist,yDist;
      
      // select a corner of the sector such that we get the max grav power
      if( i * SECTOR_SIZE > MAX_COORDINATE/2 )
        xDist = (double)(MAX_COORDINATE/2 - i * SECTOR_SIZE + SECTOR_SIZE/8);
      else
        xDist = (double)(MAX_COORDINATE/2 - (i+1) * SECTOR_SIZE + SECTOR_SIZE/8);
      if( i * SECTOR_SIZE > MAX_COORDINATE/2 )
        yDist = (double)(MAX_COORDINATE/2 - j * SECTOR_SIZE + SECTOR_SIZE/8);
      else
        yDist = (double)(MAX_COORDINATE/2 - (j+1) * SECTOR_SIZE + SECTOR_SIZE/8);
          
      double dist = sqrt( xDist * xDist + yDist * yDist );
      double angle = atan( yDist / xDist );
      double accel = _starCore->g() / (dist * dist);
      
      // take a moment to observe if this sector is "near" the center
      if(accel > .001) _sectors[i][j].nearCenter = true;
      else _sectors[i][j].nearCenter = false;
      
      // divide acceleration into x and y components
      if(xDist < 0){
        _sectors[i][j].xAccel = -accel * cos(angle);
        _sectors[i][j].yAccel = -accel * sin(angle);
      }else{
        _sectors[i][j].xAccel = accel * cos(angle);
        _sectors[i][j].yAccel = accel * sin(angle);
      }
      
      _sectors[i][j].brightness = accel * 10000000;
    }
  }
  
  // set the valid indicator for the radar to true
  _radarNew = true;
  
  // clear the radar
  for(int i=0;i<NUM_SECTORS_PER_SIDE;++i){
    for(int j=0;j<NUM_SECTORS_PER_SIDE;++j){
      _radar[i][j][_radarNew].visibility = 0;
      _radar[i][j][!_radarNew].visibility = 0;
      _radar[i][j][_radarNew].detectability = 0;
      _radar[i][j][!_radarNew].detectability = 0;
    }
  }
}



double goof(){
  return 4 * sin( 10000 * ( 1 + ( 1000.0 * rand() / (RAND_MAX+1.0) ) ) );
}



int delay(){
  Uint time = SDL_GetTicks();
  _time = time;
  
  // so we can know our fps
  if(time / 1000 >= _secCountStart + 2){
    _fps = (double)_frames / (double)(time / 1000 - _secCountStart);
    _ups = (double)_updates / (double)(time / 1000 - _secCountStart);
    
    _secCountStart = time / 1000;
    
    #ifdef DEBUG_UPDATE
    fprintf(stderr,"New starting second is %d, %d updates.\n",_secCountStart,_frames);
    #endif
    
    _frames = 0;
    _updates = 0;
  }
  
  int iterations = time - lastMSec;
  lastMSec = time;
  
  #ifdef DEBUG_UPDATE
  fprintf(stderr,"DELAY: Selected %d iterations.\n",iterations);
  if(iterations < 0){
    fprintf(stderr,"ERROR - DELAY: Iterations < 0.\n");
    return 0;
  }
  #endif
  
  if(iterations > 100) return 100;
  
  return iterations;
}



void handleNetTraffic(){
  #ifdef DEBUG_NET_TRAFF
  fprintf(stderr,"Checking for incoming network messages.\n");
  #endif
  
  // network stuff
  
  #ifdef DEBUG_NET_TRAFF
  fprintf(stderr,"Done checking for network messages.\n");
  #endif
}



// function to hide the various stages of the bulk object updating
void objectUpdateWrapper(){
  objecttype *oPtr;
    
  // *********************************************************************
  // basic updates
  
  oPtr = _objects;
  while( oPtr ){
    // apply dx and dy, apply gravity, watch map edges, determine sector
    // update returns the pointer to the next object
    oPtr = oPtr->update();
  }
  
  oPtr = _objects;
  while( oPtr ){
    // add to sector, look for collisions, resolve collisions (and death)
    // sectorUpdate returns the pointer to the next object, when an object 
    //   is moved to the dead list, meddling with next/prev pointers happens
    oPtr = oPtr->sectorUpdate();
  }
  
  // ***********************************************************************
  // special updates (mostly AI updates) must always be done after the
  //   sectors are totally populated
  
  oPtr = _objects;
  while( oPtr ){
    // apply unusual effects, such as those of AI
    // will advance the pointer as needed
    oPtr = oPtr->specialUpdate();
  }
}



// function that updates objects
void updateObjects(int iterations){
  
  for(int i=0;i<iterations;++i){
    
    // *********************************************************************
    // merge objects from the _newObjects list into the _objects list
    
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"Adding the new.\n");
    #endif
    
    // delete dead objects
    objecttype *oPtr = _newObjects;
    while(oPtr != NULL){
      _newObjects = oPtr;
      oPtr = oPtr->getNext();
      
      _newObjects->addToList();
      
      // keep the object count accurate
      ++_numObj;
    }
    _newObjects = NULL;
    
    // *********************************************************************
    // deal with objects
    
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"Starting object updates.\n");
    fprintf(stderr,"About to update _starCore.\n");
    #endif
    
    // get the star in there first so everything can run into it
    _starCore->specialUpdate();
    
    // function to hide the various stages of the bulk object updating
    objectUpdateWrapper();
    
    // *********************************************************************
    // particle systems
    
    updateDust();
    updateFlame();
    
    // *********************************************************************
    // player
    
    // update the _player
    if(_player)
      _player->specialUpdate();
    
    // *********************************************************************
    // delete objects in the _deadObjects list
    
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"Deleting the dead.\n");
    #endif
    
    // delete dead objects
    oPtr = _deadObjects;
    while(oPtr != NULL){
      _deadObjects = oPtr;
      oPtr = oPtr->getNext();
      if(_deadObjects == (objecttype*)_playerShip){
        // need to be careful because _player has a pointer to this ...
        // ... ship we want to delete
        if(_player)
          _player->makeTarget(NULL);
        
        _playerShip = NULL;
      }
      
      delete _deadObjects;
      
      // keep the object count accurate
      --_numObj;
    }
    _deadObjects = NULL;
    
    // *********************************************************************
    
    ++_timestamp;
    ++_updates;

    if( _timestamp % AI_UPDATE_DIVISOR == 0 ){    // AI update on some physics updates
      _radarNew = !_radarNew;                     // radar "buffer" flip
      for(int i=0;i<NUM_SECTORS_PER_SIDE;++i){    // clear the radar
        for(int j=0;j<NUM_SECTORS_PER_SIDE;++j){
          _radar[i][j][_radarNew].visibility = 0;
          _radar[i][j][_radarNew].detectability = 0;
        }
      }
    }
    
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"Finished object updates.\n");
    #endif
  }
}



// updates positions, calls AI stuff, does all the work, yadda yadda
void update(int iterations){
  
  #ifdef DEBUG_UPDATE
  fprintf(stderr,"Update.\n");
  #endif
  
  // *********************************************************************
  // create aliens
  
  /*if(lastAF_MSec == 0 || (lastMSec - lastAF_MSec > 5000)){
    lastAF_MSec = lastMSec;
    createAlienFighters();
  }
  
  if(lastAR_MSec == 0 || (lastMSec - lastAR_MSec > 160000)){
    lastAR_MSec = lastMSec;
    createAlienRambos();
  }*/
  
  if(lastAW_MSec == 0 || (lastMSec - lastAW_MSec > 3000)){
    lastAW_MSec = lastMSec;
    createAlienWanderers();
  }
  
  if(lastAL_MSec == 0 || (lastMSec - lastAL_MSec > 60000)){
    lastAL_MSec = lastMSec;
    createAlienLuas();
  }
  
  // *********************************************************************
  // send player updates (at proper times)
  
  // SEND ANY NEEDED UPDATES HERE
  
  // *********************************************************************
  // deal with all incoming data (from the network)
  
  handleNetTraffic();
  
  // *********************************************************************
  // the menu
  
  updateMenu();
  
  // *********************************************************************
  // things that are only updated when the game is unpaused
  
  if( !paused() ){
  
    // *******************************************************************
    // viewpoint rotation at a certain maximum rate
    
    if(_sceenRotation != _goalSceenRotation){
      float change = (float)iterations / 2.7;
      float difference = _sceenRotation - _goalSceenRotation;
      
      if( fabsf(difference) <= change){
        _sceenRotation = _goalSceenRotation;
      }else{
        if(difference > 360) _sceenRotation -= 360;
        if(difference < 0) _sceenRotation += 360;
        
        difference = _sceenRotation - _goalSceenRotation;
        
        if(difference >= 180) _sceenRotation += change;
        else if(difference < 180) _sceenRotation -= change;
      }
    }
  
    // *******************************************************************
    
    updateObjects( iterations );
  }
  
  // *********************************************************************
  // deal with the stars
  
  {
    int todo;
    if( _fps > 50.0 )
      todo = NUM_STARFIELDS;
    else
      todo = (int)((float)(NUM_STARFIELDS * _fps) / 50.0);
    if( todo < 1 )
      todo = 1;
    
    for(int i=0;i<todo;++i)
      _stars[i]->update();
  }
  
  #ifdef DEBUG_UPDATE
  fprintf(stderr,"End update.\n");
  #endif
}

