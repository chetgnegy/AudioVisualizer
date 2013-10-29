#ifndef KSMODEL_H_
#define KSMODEL_H_

#include <list>
#include "DigitalFilter.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 44100.0
#endif

class KarplusStrongString {
 public:
  // The fade in time for a single note
  static const double kAttackTime = 160.0;

  // The fade out time for a single note
  static const double kDecayTime = 5000.0;

  // Amplitude below which we can ignore the signal
  static const double kCutoffAmplitude = .001;

  // The length of the buffer -- supports notes of 20Hz
  // or greater at Fs = 441000Hz
  static const int kStandardBufferLength = 882;

  //The maximum length of any note (depends on frequency and feedback)
  int max_length_;

  // A default constructor. Doesn't do anything interesting. All parameters 
  // are set to zero, buffer is filled with noise.
  KarplusStrongString();
  // A constructor that allows for feedback specification
  KarplusStrongString(double pitch, double amplitude, double feedback);

  ~KarplusStrongString();

// Advances the read and write pointer by one element. Wrapping occurs so that it never
// goes out of bounds. A factor 'feedback' can be used to alter the current sample
// A 1 is returned if it was successful, and a 0 is returned if the note has decayed to nothing  
  int advance(double feedback);
  int advance(void);  //feedback = 0

// Pulls current sample from the location of the read pointer
  double current(void);

  // The number of samples that the note has been playing
  int note_length_;

  // The samples
  double samples_[kStandardBufferLength];

 private:
  int delay_;
  int front_;
  int back_;
  double feedback_;

};

class KarplusStrongInstrument {
 public:
  //A constructor for an instrument. Add KarplusStrongStrings to this using enqueue.
  KarplusStrongInstrument();

  ~KarplusStrongInstrument();

// Adds an additional note to the instrument. The approximate maximum polyphony
// can be set here. If the max limit is already playing, we schedule an old note
// to end soon.
  void enqueue(KarplusStrongString * ks_string);

//Performs a convolution with the resonant body and calculates the next sample.
//This is an implementation of Julius Smith's coupled string model seen here:
//https://ccrma.stanford.edu/~jos/pasp/Two_Ideal_Strings_Coupled.html
  double play(void);

  //Sets and activates a filterback for this instrument
  void set_filter_bank(FilterBank *filters);

//The notes that are currently in the instrument
  std::list<KarplusStrongString *> notes_;

 private:
  //The parallel filters
  FilterBank *filter_;
  //The current sample at the read pointer
  int current_;
};

#endif
