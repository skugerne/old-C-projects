#ifndef SHOT_HH
#define SHOT_HH



enum shotnametype { SHOT_WEAK,
                    SHOT_MED,
                    SHOT_BIG,
                    SHOT_SUPER };

enum shotcounttype { SHOT_SINGLE,
                     SHOT_DOUBLE,
                     SHOT_TRIPPLE,
                     SHOT_QUAD };
                   
enum shotmodtype { SHOT_NORM,
                   SHOT_FAST,
                   SHOT_VERY_FAST,
                   SHOT_HEAVY,
                   SHOT_FAST_HEAVY,
                   SHOT_PROX,
                   SHOT_LONG,
                   SHOT_FAST_LONG };



shotnametype shotnametypeFromString(const char*);
shotcounttype shotcounttypeFromString(const char*);
shotmodtype shotmodtypeFromString(const char*);



// the virtual class definition for the asteriods
class shottype : public objecttype {
  public:
    shottype(objecttype*,double,shotnametype,shotcounttype,shotmodtype);
    
    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcollisiontype);
    void destroy(){return;}
    
    // get functions
    double getWarhead(objectcollisiontype);
  protected:
    double mSecLeft;
    
    double warhead;
    
    shotnametype shotName;
    shotmodtype shotMod;
    
    void explode();
};



#endif

