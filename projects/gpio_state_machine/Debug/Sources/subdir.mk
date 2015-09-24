################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/dolinay/2015/git/utb_frdm_vyuka/drivers/gpio/drv_gpio.c \
D:/dolinay/2015/git/utb_frdm_vyuka/programs/gpio_state_machine/gpio_state_machine.c 

OBJS += \
./Sources/drv_gpio.o \
./Sources/gpio_state_machine.o 

C_DEPS += \
./Sources/drv_gpio.d \
./Sources/gpio_state_machine.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/drv_gpio.o: D:/dolinay/2015/git/utb_frdm_vyuka/drivers/gpio/drv_gpio.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"../Sources" -I"../Includes" -I"../../../drivers/gpio" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Sources/gpio_state_machine.o: D:/dolinay/2015/git/utb_frdm_vyuka/programs/gpio_state_machine/gpio_state_machine.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"../Sources" -I"../Includes" -I"../../../drivers/gpio" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


