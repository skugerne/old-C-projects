#ifndef SHIP_HH
#define SHIP_HH



class shiptype : public objecttype {
  public:
    shiptype(shipnametype,double,double);
    
    void draw();
    
    objecttype* specialUpdate();
    
    // an input for the effects of collisions
    void collisionEffect(double,objectcategorytype);
    void destroy();
    
    // expects a string of known legth that dictates actions
    void provideInput(Uchar*);
    
    double getWarhead(objectcategorytype);
  protected:
    shipnametype mode;
    
    bool turningLeft;
    bool turningRight;
    double turnRate;
    void falseifyControls();
    
    // are we destroyed?  (crude)
    bool destroyed;
    
    //----------------------------------------------------------------------
    // primary weapons
    
    int numWeapons;
    bool weaponOn[MAX_SHIP_WEAPONS];           // are they trying to fire?
    double weaponHeat[MAX_SHIP_WEAPONS];        // how hot are they
    double weaponMaxHeat[MAX_SHIP_WEAPONS];     // >= can no longer fire
    double weaponHeatUp[MAX_SHIP_WEAPONS];      // heat per shot
    double weaponCoolDown[MAX_SHIP_WEAPONS];    // each update
    Uint weaponLastFire[MAX_SHIP_WEAPONS];     // a certain refire
    Uint weaponReloadTime[MAX_SHIP_WEAPONS];   // (ms) min downtime between shots
    
    //----------------------------------------------------------------------
    // special weapons
    
    int numSpecial;
    bool specialOn[MAX_SHIP_SPECIALS];
    
    //----------------------------------------------------------------------
    // sensor and scanner
    
    void setSensor(double,double,double,char*);
    void setScanner(double,double,double,char*);
    
    double senStrength, gSenStrength, scanStrength;
    double scanNoise;
    double senPowerDraw, scanPowerDraw;
    char *senName, *scanName;
    
    //----------------------------------------------------------------------
    // shield and screen
    
    void setShield(double,double,double,char*);
    void setScreen(double,double,double,char*);
    
    double shldStrength, shldMaxStrength, shldRecharge, scrnStrength;
    double shldPowerDraw, shldChargeDraw, scrnPowerDraw;
    char *shldName, *scrnName;
    
    double shieldGlow;
    bool haveShield;
    void drawShield();
    
    //----------------------------------------------------------------------
    // battery, reactor, and fuel tanks
    
    double powerGenerated;    // in
    double powerRequested;    // out
    double powerAvailable;    // out / in
    
    //----------------------------------------------------------------------
    // thruster and engine
    
    bool engineOn;
    thrusteractiontype thrusterActivity;
    
    double engineThrust;
    double engineGlow, thrusterGlow;
    
    bool haveEngine, haveThruster;
    
    //----------------------------------------------------------------------
    // status bar drawing
    
    void drawVerticalBar(double,double,double,double,double,double,double,double);
};



#endif
