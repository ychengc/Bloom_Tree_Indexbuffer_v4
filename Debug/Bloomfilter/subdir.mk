################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Bloomfilter/MurmurHash3.cpp \
../Bloomfilter/bloom.cpp 

OBJS += \
./Bloomfilter/MurmurHash3.o \
./Bloomfilter/bloom.o 

CPP_DEPS += \
./Bloomfilter/MurmurHash3.d \
./Bloomfilter/bloom.d 


# Each subdirectory must supply rules for building sources it contributes
Bloomfilter/%.o: ../Bloomfilter/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


