#ifndef STARFIELD_HH
#define STARFIELD_HH



// the class definition for starfields
class starfieldtype {
  public:
    starfieldtype(int,float,float);
  
    void update();
    void draw();
  protected:
    int maxStars;
    
    // x, y, color
    float stars[512][3];
    float speed;
    
    // for star blurs
    float xChange, yChange;
    
    float lastX, lastY;
  
    float maxGoodCoord;
};



#endif
