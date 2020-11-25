#ifndef PLAYERTYPE_HH
#define PLAYERTYPE_HH



// this class is just a wrapper for the player, providing key-binding ...
// ... and network packet generation services
class playertype : public objecttype {
  public:
    playertype();
    
    void makeTarget(shiptype*);
    
    // sensor, scanner, ring...
    void draw();
    
    objecttype* specialUpdate();
    void addShake(float more){shake += more;}
    
    // here because objecttype says so
    void collisionEffect(double,objectcollisiontype){return;}
    double getWarhead(objectcollisiontype){return 0.0;}
    
    // makes string that holds commands, pass it to _playerShip
    Uchar* makeCommandString();
    
    void setEngineKey(int,int);
    void setLeftRotKey(int,int);
    void setRightRotKey(int,int);
    void setThrusterKey(int,int,int);
    void setWeaponKey(int,int,int);
    void setSpecialKey(int,int,int);
    void setMarkerKey(int,int);
    void setMessageKey(int,int);
    int getEngineKey(int);
    int getLeftRotKey(int);
    int getRightRotKey(int);
    int getThrusterKey(int,int);
    int getWeaponKey(int,int);
    int getSpecialKey(int,int);
    int getMarkerKey(int);
    int getMessageKey(int);
  protected:
    shiptype *target;
    bool targetDead;
    Uint targetDeadAt;     // when did the target become dead?
    
    // various I/O dodads
    void drawRing();
    void drawRingArrows();
    void drawRingStar();
    void drawSensor();
    void drawScanner();
  
    int engineKey[2];
    int leftRotKey[2];
    int rightRotKey[2];
    
    // thrusteractiontype is the array index
    int thrusterKey[4][2];
    
    int weaponKey[MAX_SHIP_WEAPONS][2];
    int specialKey[MAX_SHIP_SPECIALS][2];
    double specialLastChanged[MAX_SHIP_SPECIALS];
    
    int markerKey[2];
    double markerLastChanged;
    int messageKey[2];
    double messageLastChanged;
    
    void doShake();
    float shake, shakeTimeShift, xShakeShift, yShakeShift;
};



#endif
