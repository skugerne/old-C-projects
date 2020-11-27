#include "main.hpp"



// resets things that need to be reset accross games
// should always set _netStatus to "new" value before calling
void resetForNewGame(){
  #ifdef DEBUG
  fprintf(stderr,"Setting things in resetForNewGame().\n");
  #endif
  
  semiResetForNewGame();
  
  // the "reset" part ... we replace the things we deleted
  initAsteriods();
  initPlayer();
  
  #ifdef DEBUG
  fprintf(stderr,"Done setting things in resetForNewGame().\n");
  #endif
}



// resets/unsets things that need to be reset/unset accross games, leaves ...
// ... game in a state suitable for downloading a new game status (multiplayer)
// should always set _netStatus to "new" value before calling
void semiResetForNewGame(){
  #ifdef DEBUG
  fprintf(stderr,"Setting things in semiResetForNewGame().\n");
  #endif
  
  // kill current networking
  networkShutdown();
  
  // restart networking if appropriate
  if(_netStatus == GAME_HOST){
    networkStartupHost();
  }else if(_netStatus == GAME_HOST){
    networkStartupJoin();
  }else if(_netStatus != GAME_SOLO){
    fprintf(stderr,"ERROR - invalid _netStatus at semiResetForNewGame().\n");
    fprintf(stderr,"WARNG - defaulting _netStatus to GAME_SOLO.\n");
    _netStatus = GAME_SOLO;
  }
  
  _level = 1;
  _numObj = 0;
  
  if(_deadObjects) recursiveDelete(_deadObjects);
  if(_objects) recursiveDelete(_objects);
  // these two automatically delete _playerShip and _otherPlayerShip if they exist
  
  _objects = NULL;
  _deadObjects = NULL;
  _playerShip = NULL;
  _otherPlayerShip = NULL;
  
  // _player has a pointer to _playerShip that needs NULLing
  if(_player) _player->makeTarget(NULL);
  
  // otherwise people looking at the sectors map (such as _player's GUI) ...
  // ... will access deleted objects, which is bad
  for(int i=0;i<NUM_SECTORS_PER_SIDE;++i)
    for(int j=0;j<NUM_SECTORS_PER_SIDE;++j)
      _sectors[i][j].first = NULL;
  
  #ifdef DEBUG
  fprintf(stderr,"Done setting things in semiResetForNewGame().\n");
  #endif
}



// not real super efficient, but easy to do and rarely called
void recursiveDelete(objecttype *oPtr1){
  objecttype *oPtr2 = oPtr1->getNext();
  if(oPtr2) recursiveDelete(oPtr2);
  delete oPtr1;
}



// given the coordinates (which imply distance from grav source) provide
//   the dx/dy to place it into an orbit
void intoOrbit(double x, double y, double &dx, double &dy){
  double xdist = x - MAX_COORDINATE/2;
  double ydist = y - MAX_COORDINATE/2;
  double dist = sqrt(xdist*xdist + ydist*ydist);
  
  double orb_per = 2 * M_PI * pow(dist,1.5) / sqrt(GRAVITY_CONST);
  double orb_rate = DT * M_PI * 2 * dist / orb_per;
  double angle = atan(ydist/xdist);
  
  if( xdist > 0 ){
    dx = -sin(angle) * orb_rate;
    dy = cos(angle) * orb_rate;
  }else{
    dx = sin(angle) * orb_rate;
    dy = -cos(angle) * orb_rate;
  }
  
  /*
  for(int i=0;i<19;++i){
    x = 10.0 * cos( 2 * M_PI * ((double)(i+1) / 19.0));
    y = 10.0 * sin( 2 * M_PI * ((double)(i+1) / 19.0));
    dist = sqrt(x*x + y*y);
    angle = atan(y/x);
    dx = sin(angle) * 10.0;
    dy = cos(angle) * 10.0;
    printf("%d: x%f y%f dist%f angle%f dx%f dy%f\n",i,x,y,dist,angle/M_PI*180.0,dx,dy);
  }
  exit(0);
  */
}



// based on _level, add proper asteriods to game
void initAsteriods(){
  int number = 160 + _level * 40;
  double massMod = .95 + (double)_level * 0.05;
  
  srand(time(0));
  
  for(int i=0;i<number;++i){
    double extradist =
      MAX_COORDINATE / 4000.0 * ( (double)rand() / ( (double)RAND_MAX+1.0 ) );
    extradist *= extradist * extradist;
    
    double distance =
      MAX_COORDINATE / 10.0 + MAX_COORDINATE / 250.0 * ( (double)rand() / ( (double)RAND_MAX+1.0 ) );
    double angle =
      2.0 * M_PI * ( (double)rand() / ( (double)RAND_MAX+1.0 ) );
    
    distance += extradist;
    double x = MAX_COORDINATE/2 + distance * sin(angle);
    double y = MAX_COORDINATE/2 + distance * cos(angle);
    
    double mass = 10 +
      (massMod * MAX_ASTERIOD_MASS - 10) * ( (double)rand() / ( (double)RAND_MAX+1.0 ) );
    
    double dx,dy;
    intoOrbit(x,y,dx,dy);
    
    roidtype *rPtr = new roidtype(x,y,dx,dy,mass);
    rPtr->addToNewList();
  }
}



// initializes player variables (including the playertype _player)
void initPlayer(){
  #ifdef DEBUG_SHIPTYPE
  fprintf(stderr,"Initializing _player and _playerShip.\n");
  #endif
  
  // make the player if it doesn't exist (input stuff, IO stuff, viewpoint, etc)
  if(!_player) _player = new playertype();
  
  // make ship and attach player to it
  _playerShip = new shiptype(SHIP_BATTLE, _time * 4967 % MAX_COORDINATE + 5 , _time * 6991 % MAX_COORDINATE + 5);
  _playerShip->addToNewList();
  _player->makeTarget(_playerShip);
  
  #ifdef DEBUG_SHIPTYPE
  #ifdef DEBUG_OBJECTTYPE
  fprintf(stderr,"Done initializing _player and _playerShip (%d).\n",_playerShip->getID());
  #else
  fprintf(stderr,"Done initializing _player and _playerShip (for ID #define DEBUG_OBJECTTYPE).\n");
  #endif
  #endif
}



// when make == true, initializes otherPlayer variables
// when make == false, cleans up other player
void initOtherPlayer(bool make){
  if(make){
    #ifdef DEBUG_SHIPTYPE
    fprintf(stderr,"Initializing _otherPlayerShip.\n");
    #endif
    
    _otherPlayerShip = new shiptype(SHIP_BATTLE, _time * 4967 % MAX_COORDINATE + 50 , _time * 6991 % MAX_COORDINATE + 50);
    _otherPlayerShip->addToNewList();
    
    #ifdef DEBUG_SHIPTYPE
    #ifdef DEBUG_OBJECTTYPE
    fprintf(stderr,"Done initializing _otherPlayerShip (%d).\n",_otherPlayerShip->getID());
    #else
    fprintf(stderr,"Done initializing _otherPlayerShip (for ID #define DEBUG_OBJECTTYPE).\n");
    #endif
    #endif
  }else{
    if(_otherPlayerShip)
      delete _otherPlayerShip;
      
    _otherPlayerShip = NULL;
  }
}



void createFlame(double x, double y, double dx, double dy){
  // "validEnd" is on invalid data
  // when "validEnd" == "validStart" , nothing needs to be done
  // when "validEnd" == "validStart" + 1 , everything except "validEnd" needs doing
  
  float xShift = x - *_x;
  float yShift = y - *_y;
  
  // only create flame rather near the player
  if(xShift * xShift + yShift * yShift > _virtualPosX * _virtualPosX * 2) return;
  
  particletype *pPtr = &_flame.particles[_flame.validEnd];
  
  // write over whatever junk is at _flame.validEnd with new stuff
  pPtr->x = x;
  pPtr->y = y;
  pPtr->dx = dx;
  pPtr->dy = dy;
  
  // random life span
  float factor = (float)rand() / ( (float)RAND_MAX+1.0 );
  factor *= factor;
  factor *= factor;
  pPtr->msLeft = 200 + (int)(100.0 * factor);
  
  // increment and wrap around _flame.validEnd
  ++_flame.validEnd;
  if(_flame.validEnd >= NUM_FLAME) _flame.validEnd = 0;
  
  // push along _flame.validBegin as needed
  if(_flame.validEnd == _flame.validBegin) ++_flame.validBegin;
  if(_flame.validBegin >= NUM_FLAME) _flame.validBegin = 0;
  
  #ifdef DEBUG_PARTICLES
  fprintf(stderr,"Created new flame, now %d -> %d.\n",_flame.validBegin,_flame.validEnd);
  #endif
}



void createDust(double x, double y, double dx, double dy, double speedMod){
  // "validEnd" is on invalid data
  // when "validEnd" == "validStart" , nothing needs to be done
  // when "validEnd" == "validStart" + 1 , everything except "validEnd" needs doing
  
  float xShift = x - *_x;
  float yShift = y - *_y;
  
  // only create dust rather near the player
  if(xShift * xShift + yShift * yShift > _virtualPosX * _virtualPosX * 8) return;
  
  for(int i=0;i<6;++i){
    
    particletype *pPtr = &_dust.particles[_dust.validEnd];
    
    // write over whatever junk is at _dust.validEnd with new stuff
    pPtr->x = x;
    pPtr->y = y;
    
    float factor = (float)rand() / ( (float)RAND_MAX+1.0 ) - 0.5;
    float angle = 2 * M_PI * ( (float)rand() / ( (float)RAND_MAX+1.0 ) );
    
    pPtr->dx = dx + speedMod * factor * cos(angle);
    pPtr->dy = dy + speedMod * factor * sin(angle);
    
    // random life span
    factor = (float)rand() / ( (float)RAND_MAX+1.0 );
    pPtr->msLeft = 400 + (int)(2600.0 * factor);
    
    // increment and wrap around _dust.validEnd
    ++_dust.validEnd;
    if(_dust.validEnd >= NUM_FLAME) _dust.validEnd = 0;
    
    // push along _dust.validBegin as needed
    if(_dust.validEnd == _dust.validBegin) ++_dust.validBegin;
    if(_dust.validBegin >= NUM_FLAME) _dust.validBegin = 0;
  }
  
  #ifdef DEBUG_PARTICLES
  fprintf(stderr,"Created new dust, now %d -> %d.\n",_flame.validBegin,_flame.validEnd);
  #endif
}



void createAlienFighters(){
  objecttype *oPtr;
  
  // some big primes ... http://www.utm.edu/research/primes/lists/small/1000.txt
  oPtr = new alienfightertype( _time * 4967 % MAX_COORDINATE , _time * 6991 % MAX_COORDINATE );
  oPtr->addToNewList();
}



void createAlienRambos(){
  objecttype *oPtr;
  
  // some big primes ... http://www.utm.edu/research/primes/lists/small/1000.txt
  oPtr = new alienrambotype( _time * 7789 % MAX_COORDINATE , _time * 571 % MAX_COORDINATE );
  oPtr->addToNewList();
}



void createAlienWanderers(){
  objecttype *oPtr;
  
  // some big primes ... http://www.utm.edu/research/primes/lists/small/1000.txt
  oPtr = new alienwandertype( _time * 7577 % MAX_COORDINATE , _time * 3203 % MAX_COORDINATE );
  oPtr->addToNewList();
}



void createAlienLuas(){
  objecttype *oPtr;

  float x = _player->x() + 400 - (_time * 13 % 800);
  if(x > MAX_COORDINATE) x = MAX_COORDINATE-1;
  if(x < 0) x = 0;
  float y = _player->y() + 400 - (_time * 27 % 800);
  if(y > MAX_COORDINATE) y = MAX_COORDINATE-1;
  if(y < 0) y = 0;
  
  oPtr = new alienluatype(x,y);
  oPtr->addToNewList();
}
