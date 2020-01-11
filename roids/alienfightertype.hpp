#ifndef ALIEN_RAMBO_HH
#define ALIEN_RAMBO_HH



class alienfightertype : public alientype {
  public:
    alienfightertype(double,double);
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
