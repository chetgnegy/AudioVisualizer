/*
 * dfilt.h
 *
 *  Created on: Oct 14, 2013
 *      Author: chetgnegy
 */

#ifndef DFILT_H_
#define DFILT_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <list>
#include <iostream>
#include "complex.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 44100.0
#endif

#ifndef TWOPI
#define TWOPI 6.2831853072
#endif

//       All numerical stuff from here:
//
// Tunable and Variable Passive Digital Filters
//      for Multimedia Signal Processing
//                 H. K. Kwan

class DigitalFilter {
 public:
  DigitalFilter(double, double, double);
  ~DigitalFilter();
  void calculate_coefficients();
  void tick(complex);
  complex most_recent_sample(void);
  double dc_gain(void);
  double gain_;

 protected:
  double b_[3];  //The coefficients for the numerator
  double a_[3];  //The coefficients for the denominator
  complex x_past_[3];  //The previous unfiltered values
  complex y_past_[3];  //The previous outputs
  double corner_frequency_;  //Entered in Hz
  double Q_;

};

DigitalFilter::DigitalFilter(double center, double Q, double gain) {
  gain_ = gain;
  Q_ = Q;
  corner_frequency_ = TWOPI * center;  //Convert to rads/sec
  x_past_[0] = 0;
  x_past_[1] = 0;
  x_past_[2] = 0;
  y_past_[0] = 0;
  y_past_[1] = 0;
  y_past_[2] = 0;
}

DigitalFilter::~DigitalFilter() {
}

void DigitalFilter::calculate_coefficients() {
}

//Advances the filter a single
void DigitalFilter::tick(complex in) {
  x_past_[2] = x_past_[1];
  x_past_[1] = x_past_[0];
  x_past_[0] = in;
  y_past_[2] = y_past_[1];
  y_past_[1] = y_past_[0];

  y_past_[0] = -a_[1] * y_past_[1] - a_[2] * y_past_[2]
      + (b_[0] * x_past_[0] + b_[1] * x_past_[1] + b_[2] * x_past_[2]);

}

//Gets the current output of the filter.
complex DigitalFilter::most_recent_sample() {
  return y_past_[0] * gain_;
}

//Calculates the DC gain of the system. Good for normalizing with high Q or extreme corner frequency values.
double DigitalFilter::dc_gain() {
  return (b_[0] + b_[1] + b_[2]) / (a_[0] + a_[1] + a_[2]);
}

/*Bandpass Filter is a subclass of DigitalFilter*/
class DigitalBandpassFilter : public DigitalFilter {
 public:
  void calculate_coefficients();
  DigitalBandpassFilter(double f, double Q, double g)
      : DigitalFilter(f, Q, g) {
  }
  ;
};

void DigitalBandpassFilter::calculate_coefficients() {
  double bandwidth = corner_frequency_ / Q_;
  double lambda_1 = corner_frequency_ - bandwidth / 2.0;
  double lambda_2 = corner_frequency_ + bandwidth / 2.0;

  double gamma_0 = 4 * SAMPLE_RATE * SAMPLE_RATE;
  double gamma_1 = 2 * SAMPLE_RATE * (bandwidth);
  double gamma_2 = lambda_1 * lambda_2;

  double denominator = gamma_0 + gamma_1 + gamma_2;
  double alpha_1 = (gamma_0 - gamma_1 - gamma_2) / denominator;
  double alpha_2 = (gamma_0 + gamma_1 - gamma_2) / denominator;

  b_[0] = .5 * (alpha_2 - alpha_1);
  b_[1] = 0;
  b_[2] = -b_[0];
  a_[0] = 1;
  a_[1] = -(alpha_1 + alpha_2);
  a_[2] = 1 + alpha_1 - alpha_2;

}

/*Bandstop Filter is a subclass of DigitalFilter*/
class DigitalBandstopFilter : public DigitalFilter {
 public:
  void calculate_coefficients();
  DigitalBandstopFilter(double f, double Q, double g)
      : DigitalFilter(f, Q, g) {
  }
  ;
};

void DigitalBandstopFilter::calculate_coefficients() {
  double bandwidth = corner_frequency_ / Q_;
  double lambda_1 = corner_frequency_ - bandwidth / 2.0;
  double lambda_2 = corner_frequency_ + bandwidth / 2.0;

  double gamma_0 = 4 * SAMPLE_RATE * SAMPLE_RATE;
  double gamma_1 = 2 * SAMPLE_RATE * (bandwidth);
  double gamma_2 = lambda_1 * lambda_2;

  double denominator = gamma_0 + gamma_1 + gamma_2;
  double alpha_1 = (gamma_0 - gamma_1 - gamma_2) / denominator;
  double alpha_2 = (gamma_0 + gamma_1 - gamma_2) / denominator;
  double beta = -2 * (4 * pow(SAMPLE_RATE, 2) - gamma_2)
      / (4 * pow(SAMPLE_RATE, 2) + gamma_2);
  b_[0] = .5 * (2 + alpha_1 - alpha_2);
  b_[1] = beta * b_[0];
  b_[2] = b_[0];
  a_[0] = 1;
  a_[1] = -(alpha_1 + alpha_2);
  a_[2] = 1 + alpha_1 - alpha_2;

}

/*Lowpass Filter is a subclass of DigitalFilter*/
class DigitalLowpassFilter : public DigitalFilter {
 public:
  void calculate_coefficients();
  DigitalLowpassFilter(double f, double Q, double g)
      : DigitalFilter(f, Q, g) {
  }
  ;
};

void DigitalLowpassFilter::calculate_coefficients() {
  double gamma_0 = 4;
  double gamma_1 = 2 / SAMPLE_RATE * (corner_frequency_) / Q_;
  double gamma_2 = pow(corner_frequency_ / SAMPLE_RATE, 2);

  double denominator = gamma_0 + gamma_1 + gamma_2;
  double alpha_1 = (gamma_0 - gamma_1 - gamma_2) / denominator;
  double alpha_2 = (gamma_0 + gamma_1 - gamma_2) / denominator;

  b_[0] = .5 * (alpha_2 - alpha_1);
  b_[1] = 2 * b_[0];
  b_[2] = b_[0];
  a_[0] = 1;
  a_[1] = -(alpha_1 + alpha_2);
  a_[2] = 1 + alpha_1 - alpha_2;

}

class DigitalHighpassFilter : public DigitalFilter {
 public:
  void calculate_coefficients();
  DigitalHighpassFilter(double f, double Q, double g)
      : DigitalFilter(f, Q, g) {
  }
  ;
};

void DigitalHighpassFilter::calculate_coefficients() {
  double gamma_0 = 4;
  double gamma_1 = 2 / SAMPLE_RATE * (corner_frequency_) / Q_;
  double gamma_2 = pow(corner_frequency_ / SAMPLE_RATE, 2);

  double denominator = gamma_0 + gamma_1 + gamma_2;
  double alpha_1 = (gamma_0 - gamma_1 - gamma_2) / denominator;
  double alpha_2 = (gamma_0 + gamma_1 - gamma_2) / denominator;

  b_[0] = .5 * (alpha_2 + alpha_1);
  b_[1] = -2 * b_[0];
  b_[2] = b_[0];
  a_[0] = 1;
  a_[1] = -(alpha_1 + alpha_2);
  a_[2] = 1 + alpha_1 - alpha_2;

}

//A bunch of filters can be added to this. They are all used in parallel.
class FilterBank {
 public:
  FilterBank();
  ~FilterBank();
  void tick(complex);
  void add_filter(DigitalFilter *);
  complex most_recent_sample(void);
  double get_current_gain(void);
  std::list<DigitalFilter *> filters_;
  DigitalLowpassFilter *gain_control_;
 private:
  int num_filters_;
  complex current_sample_;
};

FilterBank::FilterBank() {
  gain_control_ = new DigitalLowpassFilter(10.0, 1.0, 1.0);
  gain_control_->calculate_coefficients();
  gain_control_->gain_ = 1 / gain_control_->dc_gain();
  num_filters_ = 0;
  current_sample_ = 0;

}

FilterBank::~FilterBank() {

  std::list<DigitalFilter *>::iterator list_iterator;
  list_iterator = filters_.begin();
  //Deletes all filters
  while (filters_.size() > 0 && list_iterator != filters_.end()) {
    delete (*list_iterator);
    ++list_iterator;
  }

}

void FilterBank::add_filter(DigitalFilter *f) {
  filters_.push_back(f);
  ++num_filters_;

}

void FilterBank::tick(complex in) {

  if (filters_.size() > 0) {
    std::list<DigitalFilter *>::iterator list_iterator;
    list_iterator = filters_.begin();

    complex output = 0;

    while (list_iterator != filters_.end()) {
      //The convolution with the resonant body
      (*list_iterator)->tick((in));
      output += (*list_iterator)->most_recent_sample();
      ++list_iterator;

    }

    gain_control_->tick(output.normsq());
    current_sample_ = output;
  }
}

double FilterBank::get_current_gain() {
  return gain_control_->most_recent_sample().re();
}

complex FilterBank::most_recent_sample() {
  return current_sample_;
}

#endif /* DFILT_H_ */
