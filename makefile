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
	-framework IOKit -framework Carbon -lstdc++ -lm
endif


OBJS=  KSModel.o RtAudio.o RtMidi.o fft.o dfilt.o smellovision.o

smellovision: $(OBJS)
	$(CXX) -o smellovision $(OBJS) $(LIBS) 

smellovision.o: smellovision.cpp
	$(CXX) $(FLAGS) smellovision.cpp

KSModel.o: KSModel.h KSModel.cpp 
	$(CXX) $(FLAGS) KSModel.cpp
	
RtAudio.o: RtAudio.h RtError.h RtAudio.cpp
	$(CXX) $(FLAGS) RtAudio.cpp

RtMidi.o: RtMidi.h RtError.h RtMidi.cpp 
	$(CXX) $(FLAGS) RtMidi.cpp

fft.o: fft.h complex.h fft.cpp 
	$(CXX) $(FLAGS) fft.cpp




clean:
	rm -f *~ *# *.o smellovision