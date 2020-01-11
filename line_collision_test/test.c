#include <math.h>



int main(int argc, char** argv){
  printf("1.0/0.0: %f\n",atan(1.0/0.0));
  printf("1.0/1.0: %f\n",atan(1.0/1.0));
  printf("0.0/1.0: %f\n",atan(0.0/1.0));
  printf("1.0/0.0: %f\n",1.0/0.0);
  printf("0.0/0.0: %f\n",0.0/0.0);
}
