/*
 * fourierJukebox.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: Chet Gnegy - chetgnegy@gmail.com
 *      Assignment: https://ccrma.stanford.edu/wiki/256a-fall-2013/hw1
 *
 *      Code contributions from Ge Wang
 *      Uses RtAudio Library
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include "KSModel.h"
#include "RtAudio.h"
#include "RtError.h"
#include "RtMidi.h"
#include "fft.h"
#include "complex.h"
#include "dfilt.h"

#ifdef __MACOSX_CORE__
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif


#define FORMAT RTAUDIO_FLOAT64
#define SAMPLE_RATE 44100.0
#define NUM_CHANNELS 2
#define TWOPI 6.2831853072
#define BUFFER_FRAMES 512

double global_t = 0;
DigitalLowpassFilter *anti_aliasing = new DigitalLowpassFilter(10000, 1, 1);

int callback(void *, void *, unsigned int, double, RtAudioStreamStatus, void *);
void RtAudioInitialize(RtAudio &, RtAudio::StreamParameters &,
                       RtAudio::StreamParameters &, RtAudio::StreamOptions&);
int MidiInitialize(RtMidiIn *);
void buildFilterBank(FilterBank *);
double MidiToFreq(int);

int main(int argc, char *argv[]) {

  RtAudio adac;  // instantiate RtAudio object
  RtAudio::StreamParameters input_params, output_params;
  RtAudio::StreamOptions options;
  RtAudioInitialize(adac, input_params, output_params, options);
  std::vector<unsigned char> message;
  RtMidiIn *midi_in = new RtMidiIn();
  if (MidiInitialize(midi_in)==0) return 0;

  KarplusStrongInstrument KSInst;
  FilterBank *myfilters = new FilterBank();
  buildFilterBank(myfilters);
  KSInst.set_filter_bank(myfilters);

  unsigned int num_buffer_frames = BUFFER_FRAMES;
  try {  // open a stream
    adac.openStream(&output_params, &input_params, FORMAT, SAMPLE_RATE,
                    &num_buffer_frames, &callback, (void *) &KSInst, &options);
  } catch (RtError& e) {
    std::cout << e.getMessage() << std::endl;
    exit(0);
  }

  double midi_freq = 0, midi_velo = 0;

  try {
    // opens the audio buffer
    adac.startStream();

    while (true) {

      midi_in->getMessage(&message);
      if (message.size() >= 3) {

        //Linear Velocity Scaling
        midi_velo = message[2] / 127.0;
        if (midi_velo > 0) {

          midi_freq = MidiToFreq(message[1]);

          KarplusStrongString *note = new KarplusStrongString(
              midi_freq, midi_velo, .995 - (1.0 - midi_velo) * .0005);
          KSInst.enqueue(note);

        }
      }
    }

    adac.stopStream();

  } catch (RtError& e) {
    printf("RTAudio Error\n");
  }

  // close if open
  if (adac.isStreamOpen())
    adac.closeStream();

  delete midi_in;
  return 0;

}

// This is the callback function that handles the RtAudio buffers. The KarplusStrong
// instrument is passed in through the parameter 'data'

int callback(void * outputBuffer, void * inputBuffer, unsigned int num_frames,
             double streamTime, RtAudioStreamStatus status, void * data) {
  complex *audio_data = new complex[num_frames];
  double *output_buffer = (double *) outputBuffer;
  KarplusStrongInstrument *ksInst = (KarplusStrongInstrument *) data;

  for (unsigned int i = 0; i < num_frames; ++i) {
    double newVal = ksInst->play();

    anti_aliasing->tick((.1 * newVal));

    audio_data[i] = complex(anti_aliasing->most_recent_sample());  //compute the new values
    // increment sample number
    global_t += 1.0;
  }
  //Fills those pesky other channels
  for (unsigned int i = 0; i < num_frames; ++i) {
    for (unsigned int j = 0; j < NUM_CHANNELS; ++j) {
      output_buffer[i * NUM_CHANNELS + j] = audio_data[i].re();
    }
  }

  //Make sure not to do anything with the buffer before it is sent to the output!
  CFFT::Forward(audio_data, num_frames);

  delete[] audio_data;

  return 0;
}

//Creates a bank of filters to use for our instrument
void buildFilterBank(FilterBank *myfilters) {
  DigitalBandpassFilter* bp1 = new DigitalBandpassFilter(1203, 5, .5);
  DigitalBandpassFilter* bp2 = new DigitalBandpassFilter(307, 2, .7);
  DigitalBandpassFilter* bp3 = new DigitalBandpassFilter(550, 1, .8);
  DigitalBandpassFilter* bp4 = new DigitalBandpassFilter(3430, 3, .5);
  DigitalBandpassFilter* bp5 = new DigitalBandpassFilter(8043, 1, .1);

  bp1->calculate_coefficients();
  bp2->calculate_coefficients();
  bp3->calculate_coefficients();
  bp4->calculate_coefficients();
  bp5->calculate_coefficients();

  myfilters->add_filter(bp1);
  myfilters->add_filter(bp2);
  myfilters->add_filter(bp3);
  myfilters->add_filter(bp4);
  myfilters->add_filter(bp5);

}

//Initialize RtMidi module
int MidiInitialize(RtMidiIn *midi_in) {
  // Check available ports.
  unsigned int nPorts = midi_in->getPortCount();
  if (nPorts == 0) {
    printf("No ports available!\n");
    delete midi_in;
    return 0;
  }

  midi_in->openPort(0);
  // Don't ignore sysex, timing, or active sensing messages.
  midi_in->ignoreTypes(false, false, false);
  return 1;
}

// Initializes RtAudio module. Checks for devices, enables warnings, and sets up IO
void RtAudioInitialize(RtAudio &adac, RtAudio::StreamParameters &iParams,
                       RtAudio::StreamParameters &oParams,
                       RtAudio::StreamOptions &options) {

  // check for audio devices
  if (adac.getDeviceCount() < 1) {
    printf("Audio devices not found!\n");
    exit(-1);
  }
  // let RtAudio print messages to stderr.
  adac.showWarnings(true);

  // set input and output parameters
  iParams.deviceId = adac.getDefaultInputDevice();
  iParams.nChannels = NUM_CHANNELS;
  iParams.firstChannel = 0;
  oParams.deviceId = adac.getDefaultOutputDevice();
  oParams.nChannels = NUM_CHANNELS;
  oParams.firstChannel = 0;
  anti_aliasing->calculate_coefficients();
}

double MidiToFreq(int note) {
  return pow(2, (note - 69) / 12.0) * 440.0;
}

