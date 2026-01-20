################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/IntQueueTimer.c \
../src/cpu1_init.c \
../src/gpio.c \
../src/lwip.c \
../src/main.c \
../src/platform.c 

OBJS += \
./src/IntQueueTimer.o \
./src/cpu1_init.o \
./src/gpio.o \
./src/lwip.o \
./src/main.o \
./src/platform.o 

C_DEPS += \
./src/IntQueueTimer.d \
./src/cpu1_init.d \
./src/gpio.d \
./src/lwip.d \
./src/main.d \
./src/platform.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../freertos10_xilinx_bsp_0/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


