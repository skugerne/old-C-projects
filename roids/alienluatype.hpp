#ifndef ALIEN_LUA_HH
#define ALIEN_LUA_HH

#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>



struct weapontype {
  float glow;
  bool on;
  Uint lastFired;

  Uint x;        // location on the ship model
  Uint y;
  float angle;   // location where the shots emerge relative to the location
  Uint xOffset;
  Uint yOffset;

  Uint fireDelay;
  Uint glowLimit;

  shotnametype shotname;
  shotcounttype shotcount;
  shotmodtype shotmod;
};



class alienluatype : public alientype {
  public:
    alienluatype(double,double);
    void prepWeaponPropery(Uint, const char *);
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

    lua_State *L;
};



#endif
