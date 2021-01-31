#ifndef STAR_HH
#define STAR_HH



#define STAR_RADIUS 192
#define STAR_RAYS 8*STAR_RADIUS
#define STAR_SIDES 60



// the virtual class definition for the asteriods
class startype : public objecttype {
  public:
    startype(double,double,double,double,double);
    
    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcategorytype){return;}
    void destroy(){return;}
    
    // lotsa damage for touching the star
    double getWarhead(objectcategorytype){return INSTANT_DEATH;}
  protected:
  
};



#endif
