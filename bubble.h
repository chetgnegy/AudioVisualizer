#ifndef BUBBLE_H_
#define BUBBLE_H_

#include <math.h>
#include <cstdlib>

class Bubble {
  public: 
    static const double kAscendIncrement = 0.001;
    static const double kMaxInitialVelocity = 0.0001;
    static const double kMaxChangeVelocity = .4;
    static const double kChangeDirectionFrequency = 0.007;
    Bubble(double x, double y, double z, double r);
    ~Bubble();
    void ascend(void);
    void getPosition(double *&arr);
    double distance_to(Bubble *b);
    void pop(void);
    bool is_popped(void);
    double radius_;
    
    //oscillations
    double sx_;
    double sy_;
    double sz_;
  private:
    bool popped_;
    double x_;
    double y_;
    double z_;
    double vx_;
    double vz_;
    //phases and frequencies of oscillation
    double px_;
    double py_;
    double pz_;
    double fx_;
    double fy_;
    double fz_;

    
};

#endif