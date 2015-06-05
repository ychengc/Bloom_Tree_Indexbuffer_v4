################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../btree/comparekey.cpp \
../btree/global.cpp \
../btree/run.cpp 

OBJS += \
./btree/comparekey.o \
./btree/global.o \
./btree/run.o 

CPP_DEPS += \
./btree/comparekey.d \
./btree/global.d \
./btree/run.d 


# Each subdirectory must supply rules for building sources it contributes
btree/%.o: ../btree/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


