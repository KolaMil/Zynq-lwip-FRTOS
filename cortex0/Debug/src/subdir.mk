################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/asterix_parse.c \
../src/auto_gain_control.c \
../src/cpu1_init.c \
../src/iwip.c \
../src/main.c \
../src/noise_prototype.c \
../src/platform.c \
../src/realloc_prototype.c \
../src/tcp_stack.c \
../src/ttc_timer.c \
../src/udp_stack.c 

OBJS += \
./src/asterix_parse.o \
./src/auto_gain_control.o \
./src/cpu1_init.o \
./src/iwip.o \
./src/main.o \
./src/noise_prototype.o \
./src/platform.o \
./src/realloc_prototype.o \
./src/tcp_stack.o \
./src/ttc_timer.o \
./src/udp_stack.o 

C_DEPS += \
./src/asterix_parse.d \
./src/auto_gain_control.d \
./src/cpu1_init.d \
./src/iwip.d \
./src/main.d \
./src/noise_prototype.d \
./src/platform.d \
./src/realloc_prototype.d \
./src/tcp_stack.d \
./src/ttc_timer.d \
./src/udp_stack.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../cortex0_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


