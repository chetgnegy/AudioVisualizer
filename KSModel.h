#ifndef KSMODEL_H_
#define KSMODEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <list>
#include <iostream>
#include <algorithm>    // std::min
#include "dfilt.h"

class KarplusStrongString {
 public:
  static const double kAttackTime = 160.0;
  static const double kDecayTime = 5000.0;
  //Amplitude below which we can ignore the signal
  static const double kCutoffAmplitude = 160.0;
  static const int kStandardBufferLength = 5000.0;
  int max_length_;
  KarplusStrongString();
  KarplusStrongString(double, double, double);
  ~KarplusStrongString();
  int advance(void);
  int advance(double);
  double getDelayed(int);
  double current(void);
  int note_length_;
  double samples_[kStandardBufferLength];

 private:
  int delay_;
  int front_;
  int back_;
  double feedback_;

};

class KarplusStrongInstrument {
 public:
  KarplusStrongInstrument();
  ~KarplusStrongInstrument();
  void enqueue(KarplusStrongString *);
  void set_filter_bank(FilterBank *);
  double play(void);
  std::list<KarplusStrongString *> notes_;

 private:
  FilterBank *filter_;
  int current_;
};


#endif
