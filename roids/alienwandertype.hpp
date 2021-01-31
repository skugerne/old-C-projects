#ifndef ALIEN_WANDER_HH
#define ALIEN_WANDER_HH



#ifdef DEBUG_ALIEN_WANDER
enum wanderactiontype { WANDER_FROM_STAR,
                        WANDER_FROM_NEAR,
                        WANDER_TO_NEAR };
#endif
                        


class alienwandertype : public alientype {
  public:
    alienwandertype(double,double);
    void aiupdate();
    void aifollow(objecttype*, double, double);

    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcategorytype);

    double getWarhead(objectcategorytype){return 0.0;}
  protected:
    
    #ifdef DEBUG_ALIEN_WANDER
    wanderactiontype plan;
    float targetDistances[4];
    #endif
    
    float weaponGlow[3];

    bool weaponOn[3];
    Uint weaponLastFired[3];
};



#endif
