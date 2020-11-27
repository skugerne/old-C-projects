#include "main.hpp"



Uint lastMSec;
Uint lastShipTransMSec;

// alien creation timers
Uint lastAF_MSec, lastAR_MSec, lastAW_MSec, lastAL_MSec;

#ifdef ROIDS_SMP_MODE
#define SMP_WORK_GROUP_COUNT 4
SDL_mutex* smp_work_groups[SMP_WORK_GROUP_COUNT];
#endif



void initUpdate(){
  #ifdef DEBUG_UPDATE
  fprintf(stderr,"Initializing update.\n");
  #endif
  
  lastMSec = SDL_GetTicks();
  lastShipTransMSec = lastMSec;
  
  // alien creation timers
  lastAF_MSec = lastMSec;
  lastAR_MSec = lastMSec;
  lastAW_MSec = lastMSec;
  
  // timestamp is incremented once per loop
  _timestamp = 1;
  _secCountStart = 0;
  
  // used primarily for the _fps and _ups count
  _frames = 0;
  _updates = 0;
  
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
      double accel = DT * DT * GRAVITY_CONST / (dist * dist);
      
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
  for(int i=0;i<RADAR_SIZE;++i){
    for(int j=0;j<RADAR_SIZE;++j){
      _radar[i][j][_radarNew].timestamp = 0;
      _radar[i][j][!_radarNew].timestamp = 0;
      
      _radar[i][j][_radarNew].visibility = 0;
      _radar[i][j][!_radarNew].visibility = 0;
      
      _radar[i][j][_radarNew].detectability = 0;
      _radar[i][j][!_radarNew].detectability = 0;
    }
  }
  
  #ifdef ROIDS_SMP_MODE
  for(int i=0;i<SMP_WORK_GROUP_COUNT;++i){
    smp_work_groups[i] = SDL_CreateMutex();
  }
  #endif
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



#ifdef ROIDS_SMP_MODE
int smp_sectorUpdate(void*){
  // this function depends on _numObj > SMP_WORK_GROUP_COUNT
  
  int groupsize = _numObj / SMP_WORK_GROUP_COUNT;
  int group_index = 0;
  int last_group_index = -1;
  int object_index = 0;
  objecttype *oPtr = _objects;
  
  while( oPtr ){
    group_index = object_index / groupsize;
    if( group_index >= SMP_WORK_GROUP_COUNT ) group_index = SMP_WORK_GROUP_COUNT - 1;
    
    if( group_index != last_group_index ){
      if( last_group_index != -1 )
        if( SDL_mutexV(smp_work_groups[last_group_index]) == -1 ){
          fprintf(stderr, "ERROR - Couldn't unlock mutex\n");
          exit(-1);
        }
      
      if( SDL_mutexP(smp_work_groups[group_index]) == -1 ){
        fprintf(stderr, "ERROR - Couldn't lock mutex (1)\n");
        exit(-1);
      }
      
      last_group_index = group_index;
    }
    
    ++object_index;
    
    // add to sector, look for collisions, resolve collisions (and death)
    // sectorUpdate returns the pointer to the next object, when an object 
    //   is moved to the dead list, meddling with next/prev pointers happens
    oPtr = oPtr->sectorUpdate();
  }
  
  if( SDL_mutexV(smp_work_groups[last_group_index]) == -1 ){
    fprintf(stderr, "ERROR - Couldn't unlock mutex\n");
    exit(-1);
  }
  
  return 0;
}



void smp_baseUpdate(){
  // this function depends on _numObj > SMP_WORK_GROUP_COUNT
  
  int groupsize = _numObj / SMP_WORK_GROUP_COUNT;
  int group_index = 0;
  int last_group_index = -1;
  int object_index = 0;
  objecttype *oPtr = _objects;
  
  while( oPtr ){
    group_index = object_index / groupsize;
    if( group_index >= SMP_WORK_GROUP_COUNT ) group_index = SMP_WORK_GROUP_COUNT - 1;
    //printf("Object %d => group index %d at smp_baseUpdate.\n",object_index,group_index);
    
    if( group_index != last_group_index ){
      //printf("Group index %d wanted at smp_baseUpdate.\n",group_index);
      if( last_group_index != -1 ){
        if( SDL_mutexP(smp_work_groups[group_index]) == -1 ){
          fprintf(stderr, "ERROR - Couldn't lock mutex (2)\n");
          exit(-1);
        }
        if( SDL_mutexV(smp_work_groups[last_group_index]) == -1 ){
          fprintf(stderr, "ERROR - Couldn't unlock mutex\n");
          exit(-1);
        }
      }
      last_group_index = group_index;
    }
    
    ++object_index;
    
    // add to sector, look for collisions, resolve collisions (and death)
    // sectorUpdate returns the pointer to the next object, when an object 
    //   is moved to the dead list, meddling with next/prev pointers happens
    oPtr = oPtr->update();
  }
  
  if( SDL_mutexV(smp_work_groups[last_group_index]) == -1 ){
    fprintf(stderr, "ERROR - Couldn't unlock mutex\n");
    exit(-1);
  }
}
#endif



// function to hide the various stages of the bulk object updating
// also hides the handling of optional SMP support
void objectUpdateWrapper(){
  objecttype *oPtr;  
  
  #ifdef ROIDS_SMP_MODE
  if( _numObj < 2 * SMP_WORK_GROUP_COUNT ){
  #endif
    
    // *********************************************************************
    // simple case where these is no threading
    
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
    
  #ifdef ROIDS_SMP_MODE
  }else{
    
    // *********************************************************************
    // case where there is threading
    
    if( SDL_mutexP(smp_work_groups[0]) == -1 ){
      fprintf(stderr, "ERROR - Couldn't lock mutex (3)\n");
      exit(-1);
    }
    
    SDL_Thread *sectorThread = SDL_CreateThread(smp_sectorUpdate,NULL);
    
    smp_baseUpdate();
    
    int result;
    SDL_WaitThread(sectorThread,&result);
    
  }
  #endif
  
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
    // also hides the handling of optional SMP support
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
    _radarNew = !_radarNew;   // radar "buffer" flip
    
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
  
  if(lastAF_MSec - SDL_GetTicks() > 10000){
    lastAF_MSec += 10000;
    createAlienFighters();
  }
  
  if(lastAR_MSec - SDL_GetTicks() > 160000){
    lastAR_MSec += 160000;
    createAlienRambos();
  }
  
  if(lastAW_MSec - SDL_GetTicks() > 20000){
    lastAW_MSec += 20000;
    createAlienWanderers();
  }
  
  if(lastAL_MSec - SDL_GetTicks() > 5000){
    lastAL_MSec += 5000;
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
    if( _fps > 240.0 )
      todo = NUM_STARFIELDS;
    else if( _fps < 30.0 )
      todo = 1;
    else
      todo = (int)((float)(NUM_STARFIELDS * _fps) / 250.0);
    
    for(int i=0;i<todo;++i)
      _stars[i]->update();
  }
  
  #ifdef DEBUG_UPDATE
  fprintf(stderr,"End update.\n");
  #endif
}

