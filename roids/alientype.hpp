#ifndef ALIEN_HH
#define ALIEN_HH



#define NO_TARGET 99999999.0



// container to return targeting information in
// *** FIXME *** someday return brightness, etc
struct targettype {
  float bearing;    // relation to the alien
  float distance;
  float heading;    // direction of relative movement
  float approach;   // directness of approach
  float speed;      // speed of movement, relative to this alien
};



class alientype : public objecttype {
  public:
    
  protected:
    // commands from ai
    float heading;
    bool engineOn;
    bool turningLeft;
    bool turningRight;

    // general engine things
    float turnRate;
    float enginePower;
    float maxSpeed;
    float engineGlow;

    // is it destroyed?  (crude)
    bool destroyed;
    float shieldPoints, shieldGlow;
    void drawShield();
    
    
    //----------------------------------------------------------------------
    // functions to identify targets
    // place into targettype array, return number of targets found
    // results sorted by distance
    
    // consider only range to target
    int findTargets(targettype *,int,float);
    
    // consider range and approach
    int findTargetsApp(targettype *,int,float,float);
    
    // consider range and bearing
    int findTargetsAngle(targettype *,int,float,float,float);
    
    // consider range, bearing and approach
    int findTargetsAngleApp(targettype *,int,float,float,float,float);
};



#endif
