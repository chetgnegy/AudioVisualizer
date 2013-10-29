#include "bubble.h"



Bubble::Bubble(double x, double y, double z, double r){
  x_=x; y_=y; z_=z; radius_ = r;
  
  px_ = rand()/(1.0*RAND_MAX);
  py_ = rand()/(1.0*RAND_MAX);
  pz_ = rand()/(1.0*RAND_MAX);
  fx_ = 0.009*rand()/(1.0*RAND_MAX);
  fy_ = 0.01*rand()/(1.0*RAND_MAX);
  fz_ = 0.004*rand()/(1.0*RAND_MAX);
  
  vx_ = 2 * kMaxInitialVelocity * 2 * (rand()/(1.0*RAND_MAX) - .5);
  vz_ = 2 * kMaxInitialVelocity * 2 * (rand()/(1.0*RAND_MAX) - .5);
  popped_ = false;
  
  sx_ = 1;
  sy_ = 1;
  sz_ = 1;
}

Bubble::~Bubble(){
}

void Bubble::ascend(void){
  y_ += kAscendIncrement * (rand()/(1.0*RAND_MAX)) / (.3+radius_);
  x_ += vx_;
  z_ += vz_;
  
  if (rand()/(1.0 * RAND_MAX) < kChangeDirectionFrequency){
    vx_ = vx_ * (1 + kMaxChangeVelocity * 2 *(rand()/(1.0*RAND_MAX) - .5));
    vz_ = vz_ * (1 + kMaxChangeVelocity * 2 *(rand()/(1.0*RAND_MAX) - .5));
  }
  sx_ = .1*cos(fx_* ++px_)+1;
  sy_ = .1*cos(fy_* ++py_)+1;
  sz_ = .1*cos(fz_* ++pz_)+1;
  
  if (y_ > 20) pop();
}

void Bubble::getPosition(double *& arr){
  arr[0] = x_;
  arr[1] = y_;
  arr[2] = z_;

}

double Bubble::distance_to(Bubble *b){
  double *b_pos = new double[3]; 
  b->getPosition(b_pos);
  double distsq = pow(x_ - b_pos[0],2) + pow(y_ - b_pos[1],2) + pow(z_ - b_pos[2],2);
  delete[] b_pos;
  return pow(distsq , .5);
  
}

void Bubble::pop(void){
  popped_ = true;
  //printf("POP!\n");
}
bool Bubble::is_popped(void){
  return popped_;
}