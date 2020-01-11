#ifndef ROID_HH
#define ROID_HH



// the virtual class definition for the asteriods
class roidtype : public objecttype {
  public:
    roidtype(double,double,double,double,double);
    
    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcollisiontype);
    
    // get functions
    double getWarhead(objectcollisiontype){return 0.0;}
  protected:
    void breakUp();
    
    void die(float);      // pass it how much "bang" to draw
    float hitPoints, origHitPoints;
    float deathByFade;    // used for little asteroids to peacefully go away
    
    // damage display
    float damageGlow;
    void showDamage();
    
    // rotation
    void setRandomRotation();
    float aChange;
    
    //----------------------------------------------------------------------
    // are we in bezier mode or standard mode?
    
    bool standardMode;
    void determineMode();
    
    //----------------------------------------------------------------------
    // things specific to the standard mode
    
    void initStandardMode();
    
    void drawStandard();
    
    //----------------------------------------------------------------------
    // things specific to the bezier mode
    
    void initBezierMode();
    
    void drawBezier();
    
    void calcCenteringShifts();
    void calcScale();
    float bezierValue(int,int,int);
    
    float convertXToLocal(float,float);
    float convertYToLocal(float,float);
  
    // what degree of detail should it be drawn at?
    // 120 is the largest such value
    int corners;
    int cornersPerSide;
    float scale;
    
    // six splines, sharing end points, in a loop, 10 plotted points each
    // 19th spot equals first
    float cPoints[19][2];
    float pPoints[120][2];
    
    float xCenteringShift;
    float yCenteringShift;
};



#endif
