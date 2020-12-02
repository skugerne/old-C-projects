#ifndef ALIEN_LUA_HH
#define ALIEN_LUA_HH

#ifdef LUA53
  #include <lua5.3/lua.h>
  #include <lua5.3/lualib.h>
  #include <lua5.3/lauxlib.h>
#endif

#ifdef LUA51
  extern "C" {
    #include <lua-5.1/lua.h>
    #include <lua-5.1/lualib.h>
    #include <lua-5.1/lauxlib.h>
  }

  #ifndef LUA_OK
    #define LUA_OK 0
  #endif
#endif



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
    alienluatype(double,double);
    void prepLuaTablePropery(Uint, const char *, const char *);
    void init();
    void aiupdate();
    void aifollow(objecttype*, double, double);

    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcollisiontype);

    double getWarhead(objectcollisiontype){return 0.0;}
  protected:
    
    Uint numWeapons;
    weapontype *weapons;
    
    Uint numEngines;
    enginetype *engines;

    lua_State *L;
};



#endif
