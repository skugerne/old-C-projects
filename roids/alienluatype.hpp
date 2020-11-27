#ifndef ALIEN_LUA_HH
#define ALIEN_LUA_HH



class alienluatype : public alientype {
  public:
    alienluatype(double,double);
    void aiupdate();
    void aifollow(objecttype*, double, double);

    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcollisiontype);

    double getWarhead(objectcollisiontype){return 0.0;}
  protected:
    
    float weaponGlow;

    bool weaponOn;
    Uint weaponLastFired;
};



#endif
