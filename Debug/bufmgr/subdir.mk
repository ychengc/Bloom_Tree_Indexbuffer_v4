################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../bufmgr/bufmgr.cpp \
../bufmgr/lrubuf.cpp \
../bufmgr/nobuf.cpp 

OBJS += \
./bufmgr/bufmgr.o \
./bufmgr/lrubuf.o \
./bufmgr/nobuf.o 

CPP_DEPS += \
./bufmgr/bufmgr.d \
./bufmgr/lrubuf.d \
./bufmgr/nobuf.d 


# Each subdirectory must supply rules for building sources it contributes
bufmgr/%.o: ../bufmgr/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


