################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/drv_audio_mod.c \
../Sources/drv_lcd.c \
../Sources/i2c.c \
../Sources/main.c 

OBJS += \
./Sources/drv_audio_mod.o \
./Sources/drv_lcd.o \
./Sources/i2c.o \
./Sources/main.o 

C_DEPS += \
./Sources/drv_audio_mod.d \
./Sources/drv_lcd.d \
./Sources/i2c.d \
./Sources/main.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"../Sources" -I"../Includes" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


