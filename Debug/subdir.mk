################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../RtAudio.o \
../RtMidi.o \
../fft.o \
../graphics.o \
../smellovision.o 

CPP_SRCS += \
../RtAudio.cpp \
../RtMidi.cpp \
../complex.cpp \
../fft.cpp \
../graphics.cpp \
../smellovision.cpp 

OBJS += \
./RtAudio.o \
./RtMidi.o \
./complex.o \
./fft.o \
./graphics.o \
./smellovision.o 

CPP_DEPS += \
./RtAudio.d \
./RtMidi.d \
./complex.d \
./fft.d \
./graphics.d \
./smellovision.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


