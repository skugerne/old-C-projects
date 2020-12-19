#ifndef ALIEN_LUA_HH
#define ALIEN_LUA_HH



struct weapontype {
  float glow;
  float glowCoolRate;
  bool on;
  Uint lastFired;

  float x;        // location on the ship model
  float y;
  float angle;    // angle where the shots emerge relative to the location
  float xOffset;  // offset at the given angle where shots emerge
  float yOffset;

  Uint fireDelay;
  Uint glowLimit;

  shotnametype shotname;
  shotcounttype shotcount;
  shotmodtype shotmod;
};



struct enginetype {
  Uint firingMod;    // one timestamp in this many emits fire
  Uint firingPoint;  // emit fire when the timestamp mod 'firingMod' is this
  float x;           // location on the ship model
  float y;
};



class alienluatype : public alientype {
  public:
    alienluatype(double,double,const char *);
    void aiupdate();
    void aifollow(objecttype*, double, double);

    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcollisiontype);

    double getWarhead(objectcollisiontype){return 0.0;}
  protected:
    void prepLuaTablePropery(Uint, const char *);
    void init();

    int findHot(lua_State *);
    int setEngine(lua_State *);
    int setTurnLeft(lua_State *);
    int setTurnRight(lua_State *);
    
    Uint numWeapons;
    weapontype *weapons;
    
    Uint numEngines;
    enginetype *engines;

    lua_State *L;
};



#endif
