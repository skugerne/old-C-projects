#ifndef OBJECT_HH
#define OBJECT_HH

#define INSTANT_DEATH 1000.0   // the most damage that there can be



inline float angleLimit(float a){if(a < 0) a += 360.0; else if(a >= 360.0) a -= 360.0; return a;}



// the root class of things that go bump in the night
class objecttype {
  friend void collision(objecttype*,objecttype*);
  public:
    void virtual draw() = 0;
    
    // both return next object in the list
    objecttype *update();
    objecttype *sectorUpdate();
    objecttype virtual *specialUpdate() = 0;
    
    void virtual collisionEffect(double,objectcollisiontype) = 0;
    void virtual destroy() = 0;
    
    // returns distance from given objecttype
    double distanceFrom(objecttype*);
    bool viewable(double);
    
    // _objects list management
    void addToList();
    void removeFromList();
    void addToNewList();
    void moveToDead();
    
    // get functions
    objecttype* getNext(){return next;}
    objecttype* getSecNext(){return sectorLess;}   // *** FIXME ***
    objecttype* getSecLess(){return sectorLess;}
    objecttype* getSecMore(){return sectorMore;}
    double x(){return xCoordinate;}
    double y(){return yCoordinate;}
    double* xPtr(){return &xCoordinate;}
    double* yPtr(){return &yCoordinate;}
    double dx(){return xChange;}
    double dy(){return yChange;}
    double* dxPtr(){return &xChange;}
    double* dyPtr(){return &yChange;}
    double a(){return angle;}
    double* aPtr(){return &angle;}
    double r(){return radius;}
    double m(){return mass;}
    const char* name(){return nameString;}
    double getDetectability(){return detectabilityFactor;}
    double getVisibility(){return visibilityFactor;}
    objectcollisiontype getCollisionMod(){return collisionModifier;}
    double virtual getWarhead(objectcollisiontype) = 0;
    bool getDead(){return isDead;}
    
    #ifdef DEBUG_OBJECTTYPE
    Uint getID(){return idNum;}
    #endif
  protected:
    void basicInit();
    bool isDead;
    
    #ifdef DEBUG_OBJECTTYPE
    Uint idNum;
    #endif
    
    #ifdef DEBUG_THRUST
    // thrust status output variables
    double thrustStat1, thrustStat2, thrustStat3;
    #endif
    
    // sector list and radar management
    void addToSector(sectortype*);
    void addToRadar(radartype*);
    
    void drawRadius(bool);                      // a debugging tool
    void drawLabel(bool,double,double,double);  // a debugging tool
    void drawHeading(bool);                     // a debugging tool
    
    objecttype *next;
    objecttype *prev;
    objecttype *sectorLess;
    objecttype *sectorMore;
    const char *nameString;
    
    unsigned int sectorUpdateWhen;
    bool inBounds;
    void setInBounds();
    
    void placeInSector();           // involves collision detect
    void checkSectorRecur(objecttype*);
    void checkSector(sectortype*);
    void collideWithEdges();
    void gravitate();
    void thrust(double,double,double);
    objectcollisiontype collisionModifier;
    
    int xSectorIndex;
    int ySectorIndex;
    double xCoordinate;
    double yCoordinate;
    double xChange;
    double yChange;
    
    double angle;
    double mass;
    double radius;
    
    double visibilityFactor, baseVisibility;
    double detectabilityFactor;
};



#endif
