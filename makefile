#!/bin/sh

CXX=llvm-g++-4.2
INCLUDES=

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
FLAGS=-D__UNIX_JACK__ -c
LIBS=-lasound -lpthread -ljack -lstdc++ -lm
endif
ifeq ($(UNAME), Darwin)
FLAGS=-D__MACOSX_CORE__ -c
LIBS=-framework CoreAudio -framework CoreMIDI -framework CoreFoundation \
	-framework IOKit -framework Carbon -framework OpenGL \
	-framework GLUT -framework Foundation -framework AppKit \
	-lstdc++ -lm
endif


OBJS=  RtAudio.o RtMidi.o fft.o AudioFeatures.o DigitalFilter.o KSModel.o bubble.o smellovision.o 

smellovision: $(OBJS)
	$(CXX) -o smellovision $(OBJS) $(LIBS) 

smellovision.o: smellovision.cpp graphics.h DigitalFilter.h
	$(CXX) $(FLAGS) smellovision.cpp

DigitalFilter.o: DigitalFilter.cpp DigitalFilter.h
	$(CXX) $(FLAGS) DigitalFilter.cpp

bubble.o: bubble.cpp bubble.h
	$(CXX) $(FLAGS) bubble.cpp

KSModel.o: KSModel.cpp KSModel.h DigitalFilter.h
	$(CXX) $(FLAGS) KSModel.cpp

AudioFeatures.o: AudioFeatures.cpp AudioFeatures.h
	$(CXX) $(FLAGS) AudioFeatures.cpp

fft.o: fft.h complex.h fft.cpp 
	$(CXX) $(FLAGS) fft.cpp

RtAudio.o: RtAudio.h RtError.h RtAudio.cpp
	$(CXX) $(FLAGS) RtAudio.cpp

RtMidi.o: RtMidi.h RtError.h RtMidi.cpp 
	$(CXX) $(FLAGS) RtMidi.cpp





clean:
	rm -f *~ *# *.o smellovision