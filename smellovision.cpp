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
#include <math.h>
#include <iostream>

#include "RtAudio.h"
#include "RtError.h"
#include "RtMidi.h"

#include "complex.h"

#include "KSModel.h"
#include "DigitalFilter.h"
#include "graphics.h"

#include "AudioFeatures.h"

#ifdef __MACOSX_CORE__
#include <GLUT/glut.h>
#else
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
#endif

#define FORMAT RTAUDIO_FLOAT64
#define SAMPLE_RATE 44100.0
#define NUM_CHANNELS 2
#define TWOPI 6.2831853072
#define BUFFER_FRAMES 1024

double global_t = 0;
bool g_midi = false;
DigitalLowpassFilter *g_anti_aliasing = new DigitalLowpassFilter(10000, 1, 1);
DigitalHighpassFilter *g_low_pass = new DigitalHighpassFilter(10, 1, 1);
AudioFeatures g_features = AudioFeatures(BUFFER_FRAMES);


int callback(void *, void *, unsigned int, double, RtAudioStreamStatus, void *);
void RtAudioInitialize(RtAudio &, RtAudio::StreamParameters &,
                       RtAudio::StreamParameters &, RtAudio::StreamOptions&);
int MidiInitialize(RtMidiIn *);
void buildFilterBank(FilterBank *);
void analyze_audio(int);
double MidiToFreq(int);

RtMidiIn *midi_in = new RtMidiIn();

int main(int argc, char *argv[]) {

  RtAudio adac;  // instantiate RtAudio object
  RtAudio::StreamParameters input_params, output_params;
  RtAudio::StreamOptions options;
  RtAudioInitialize(adac, input_params, output_params, options);
  
  MidiInitialize(midi_in);
  
  //Sets up our Karplus Strong instrument and filter bank
  KarplusStrongInstrument KSInst;
  FilterBank *myfilters = new FilterBank();
  buildFilterBank(myfilters);
  KSInst.set_filter_bank(myfilters);

  //Graphic Setup
  setGraphicsFeatures(&g_features);
  startGraphics(argc, argv);


  // Tells the audio stream how to open and what callback to use
  unsigned int num_buffer_frames = BUFFER_FRAMES;
  try {  // open a stream
    adac.openStream(&output_params, &input_params, FORMAT, SAMPLE_RATE,
                    &num_buffer_frames, &callback, (void *) &KSInst, &options);
  } catch (RtError& e) {
    std::cout << e.getMessage() << std::endl;
    exit(0);
  }
  
  // do our own initialization
  glInitialize();

  try {
    // opens the audio buffer
    adac.startStream();
    glutMainLoop();

    char input;
    std::cin.get(input);
    
    adac.stopStream();

  } catch (RtError& e) {
    printf("RTAudio Error\n");
  }
  // close if open
  if (adac.isStreamOpen())
    adac.closeStream();
  return 0;

}

// This is the callback function that handles the RtAudio buffers. The KarplusStrong
// instrument is passed in through the parameter 'data'
int callback(void * outputBuffer, void * inputBuffer, unsigned int num_frames,
             double streamTime, RtAudioStreamStatus status, void * data) {

  double *input_buffer = (double *) inputBuffer;
  setSoundBuffer(input_buffer, num_frames);
  double *output_buffer = (double *) outputBuffer;
  KarplusStrongInstrument *ksInst = (KarplusStrongInstrument *) data;
  
  if (g_midi){
    //Reads MIDI input
    std::vector<unsigned char> message;
    double midi_freq = 0, midi_velo = 0;
    while (true) {
        midi_in->getMessage(&message);
        if (message.size() >= 3) {
          midi_velo = message[2] / 127.0; //Linear Velocity Scaling
          if (midi_velo > 0) {
            midi_freq = MidiToFreq(message[1]);
            KarplusStrongString *note = new KarplusStrongString(
                midi_freq, midi_velo, .995 - (1.0 - midi_velo) * .0005);
            ksInst->enqueue(note);
          }
        }
        else break;
      }
  }
    
  
  double newVal = 0;
  for (unsigned int i = 0; i < num_frames; ++i) {
     newVal = ksInst->play() + input_buffer[i*NUM_CHANNELS];

    g_low_pass->tick(newVal);
    g_anti_aliasing->tick(g_low_pass->most_recent_sample());
    g_features.add_next_sample(g_anti_aliasing->most_recent_sample());  //compute the new values
    output_buffer[i * NUM_CHANNELS] = 0.05*g_anti_aliasing->most_recent_sample().re();
  
  }
  
  if (g_features.spectral_power()>30000){
    popAll();
  }
  int chroma = g_features.get_note_event();
  if (chroma != -1){
    addBubble(chroma);
  }
  
  //Fills the other channels
  for (unsigned int i = 0; i < num_frames; ++i) {
    for (unsigned int j = 1; j < NUM_CHANNELS; ++j) {
      output_buffer[i * NUM_CHANNELS + j] = output_buffer[i * NUM_CHANNELS ];
    }
  }
  
  //char input;
  //std::cin.get(input);
  return 0;
}




//Initialize RtMidi module
int MidiInitialize(RtMidiIn *midi_in) {
  // Check available ports.
  unsigned int nPorts = midi_in->getPortCount();
  if (nPorts == 0) {
    g_midi = false;
    printf("No ports available!\n");
    return 0;
  }
  g_midi = true;
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
  
}

double MidiToFreq(int note) {
  return pow(2, (note - 69) / 12.0) * 440.0;
}

//Creates a bank of filters to use for our instrument
void buildFilterBank(FilterBank *myfilters) {
  DigitalBandpassFilter* bp1 = new DigitalBandpassFilter(1203, 5, .5);
  DigitalBandpassFilter* bp2 = new DigitalBandpassFilter(307, 2, .7);
  DigitalBandpassFilter* bp3 = new DigitalBandpassFilter(550, 1, .8);
  DigitalBandpassFilter* bp4 = new DigitalBandpassFilter(3430, 3, .5);
  DigitalBandpassFilter* bp5 = new DigitalBandpassFilter(8043, 1, .1);
  
  myfilters->add_filter(bp1);
  myfilters->add_filter(bp2);
  myfilters->add_filter(bp3);
  myfilters->add_filter(bp4);
  myfilters->add_filter(bp5);
}




