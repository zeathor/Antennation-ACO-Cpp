################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Ant.cpp \
../Colony.cpp \
../Definitions.cpp \
../Harness.cpp \
../PherMap.cpp \
../RandomMersenne.cpp 

OBJS += \
./Ant.o \
./Colony.o \
./Definitions.o \
./Harness.o \
./PherMap.o \
./RandomMersenne.o 

CPP_DEPS += \
./Ant.d \
./Colony.d \
./Definitions.d \
./Harness.d \
./PherMap.d \
./RandomMersenne.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


