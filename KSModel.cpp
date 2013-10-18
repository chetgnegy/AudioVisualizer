
#include "KSModel.h"






//A default constructor. Doesn't do anything interesting. All parameters are set to zero, buffer is
//filled with noise.
KarplusStrongString::KarplusStrongString() {
  front_ = 0;
  delay_ = 0;
  for (int i = 0; i < kStandardBufferLength; ++i) {
    samples_[i] = (rand() / 1.0 * RAND_MAX) * 2.0 - 1.0;
  }
  back_ = delay_;
  feedback_ = 0;
  max_length_ = 0;
  note_length_ = 0;
}

//A constructor that allows for feedback specification
KarplusStrongString::KarplusStrongString(double pitch, double amp, double fb) {
  front_ = 0;
  delay_ = (int) (SAMPLE_RATE / static_cast<double>(pitch));
  double amplitude = std::max(0.001, amp);
  for (int i = 0; i < kStandardBufferLength; ++i) {
    samples_[i] = amplitude * (rand() / (1.0 * RAND_MAX) * 2.0 - 1.0);
  }
  back_ = delay_;
  feedback_ = std::min(fb, 1.0 - kCutoffAmplitude);
  max_length_ = round(
      delay_ * (log(kCutoffAmplitude) - log(amplitude)) / log(feedback_));
  note_length_ = 0;

}


KarplusStrongString::~KarplusStrongString() {
}

//Advances the read and write pointer by one element. Wrapping occurs so that it never
//goes out of bounds. A factor addToSample can be used to alter the current sample
//A 1 is returned if it was successful, and a 0 is returned if the note has decayed to nothing
int KarplusStrongString::advance(double add_to_sample) {
  if (note_length_ < max_length_) {
    int otherSample = (front_ - 1 + kStandardBufferLength)
        % kStandardBufferLength;
    //feedback
    double current = samples_[back_] - add_to_sample;

    //lowpass
    samples_[front_] = feedback_ * (current + samples_[otherSample]) / 2.0;

    //Update the pointers to the samples
    front_ = (front_ + 1 + kStandardBufferLength);
    back_ = (front_ - delay_);
    front_ %= kStandardBufferLength;
    back_ %= kStandardBufferLength;
    ++note_length_;
    return 1;
  } else {
    return 0;
  }
}

//Pulls a sample from the sample array with offset current - i. i can be positive or negative.
double KarplusStrongString::current(void) {
  int which_sample = (back_ + kStandardBufferLength) % kStandardBufferLength;

  if (note_length_ < KarplusStrongString::kAttackTime)  //prevents pop on attack
    return samples_[which_sample] * note_length_
        / KarplusStrongString::kAttackTime;
  else if (max_length_ - (note_length_) < KarplusStrongString::kDecayTime)  // prevents pop on decay
    return samples_[which_sample] * (max_length_ - note_length_)
        / KarplusStrongString::kDecayTime;
  return samples_[which_sample];

}

//Advances the read and write pointer without any feedback.
int KarplusStrongString::advance(void) {
  return KarplusStrongString::advance(0);
}

//A constructor for an instrument. It requires KarplusStrongStrings to be of any use.
KarplusStrongInstrument::KarplusStrongInstrument() {
  filter_ = NULL;
  current_ = 0;
}
KarplusStrongInstrument::~KarplusStrongInstrument() {
  delete filter_;
  std::list<KarplusStrongString *>::iterator list_iterator;
  list_iterator = notes_.begin();
  //Deletes all strings
  while (notes_.size() > 0 && list_iterator != notes_.end()) {
    delete (*list_iterator);
    ++list_iterator;
  }
}
// Adds an additional note to the instrument. The approximate maximum polyphony
// can be set here. If the max limit is already playing, we schedule an old note
// to end soon.
void KarplusStrongInstrument::enqueue(KarplusStrongString *k) {
  if (notes_.size() > 16) {  //max polyphony (not a hard limit)
    std::list<KarplusStrongString *>::iterator list_iterator;
    list_iterator = notes_.begin();
    //schedule first note in queue that isn't about to end to end soon

    int found_note = 0;
    do {
      //Note isn't about to end
      if ((*list_iterator)->note_length_
          < (*list_iterator)->max_length_ - KarplusStrongString::kDecayTime) {
        //force it to end soon
        (*list_iterator)->note_length_ = (*list_iterator)->max_length_
            - KarplusStrongString::kDecayTime;
        found_note = 1;
      } else {
        ++list_iterator;
      }
      //keep going until you find a note or run out
    } while (found_note == 0 && list_iterator != notes_.end());
  }
  notes_.push_back(k);
}

//Performs a convolution with the resonant body and calculates the next sample.
//This is an implementation of Julius Smith's coupled string model seen here:
//https://ccrma.stanford.edu/~jos/pasp/Two_Ideal_Strings_Coupled.html
double KarplusStrongInstrument::play() {
  double sum = 0;
  if (notes_.size() == 0)
    return 0;

  std::list<KarplusStrongString *>::iterator list_iterator;
  list_iterator = notes_.begin();
  double next_sample = 0;

  //Automatic Gain Control
  double K = 0;
  if (filter_ != NULL) {
    next_sample = filter_->most_recent_sample().re();
    K = filter_->gain_control_->most_recent_sample().re();
  }
  while (list_iterator != notes_.end()) {
    //The convolution with the resonant body
    if ((*list_iterator)->advance(next_sample) == 1)
      sum += (*list_iterator)->current();
    else {
      notes_.erase(list_iterator);
      delete *list_iterator;
    }
    ++list_iterator;
  }

  if (filter_ != NULL) {
    filter_->tick(complex(sum / pow(1 + 5 * K, 6)));  //These are some magic numbers. Change them!
  }
  return sum;
}

void KarplusStrongInstrument::set_filter_bank(FilterBank *f) {
  filter_ = f;

}

