#ifndef ALIEN_RAMBO_HH
#define ALIEN_RAMBO_HH



class alienrambotype : public alientype {
  public:
    alienrambotype(double,double);
    void aiupdate();
    void aifollow(objecttype*, double, double);

    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcategorytype);

    double getWarhead(objectcategorytype){return 0.0;}
  protected:
    
    float weaponGlow;

    bool weaponOn;
    float weaponAngle, weaponTurnRate, weaponMaxAngle, weaponMinAngle;
    bool weaponLeft;
    bool weaponRight;
    Uint weaponLastFired;
};



#endif
