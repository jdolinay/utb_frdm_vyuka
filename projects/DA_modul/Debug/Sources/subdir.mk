################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/drv_DA_mod.c \
../Sources/drv_spi.c \
../Sources/main.c 

OBJS += \
./Sources/drv_DA_mod.o \
./Sources/drv_spi.o \
./Sources/main.o 

C_DEPS += \
./Sources/drv_DA_mod.d \
./Sources/drv_spi.d \
./Sources/main.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCLOCK_SETUP=1 -I"../Sources" -I"../Includes" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


