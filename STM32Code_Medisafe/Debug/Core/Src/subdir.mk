################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/CSN_A2.c \
../Core/Src/ENDOCLEAN2000.c \
../Core/Src/MAX6675.c \
../Core/Src/Medisafe.c \
../Core/Src/Medisafe_Gpio_Config.c \
../Core/Src/Medisafe_Mode_Config.c \
../Core/Src/SystemTicks.c \
../Core/Src/UltrasonicGenerator.c \
../Core/Src/ds18b20.c \
../Core/Src/fsm_reprocesadora1.c \
../Core/Src/global_variables.c \
../Core/Src/i2c_pressure.c \
../Core/Src/main.c \
../Core/Src/nextion.c \
../Core/Src/ntc_adc.c \
../Core/Src/onewire.c \
../Core/Src/relay_module.c \
../Core/Src/rfid_wiegand.c \
../Core/Src/rs485_pressure_sensor.c \
../Core/Src/service.c \
../Core/Src/softserial.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/ticket.c \
../Core/Src/user_gpio.c \
../Core/Src/watchdog.c 

OBJS += \
./Core/Src/CSN_A2.o \
./Core/Src/ENDOCLEAN2000.o \
./Core/Src/MAX6675.o \
./Core/Src/Medisafe.o \
./Core/Src/Medisafe_Gpio_Config.o \
./Core/Src/Medisafe_Mode_Config.o \
./Core/Src/SystemTicks.o \
./Core/Src/UltrasonicGenerator.o \
./Core/Src/ds18b20.o \
./Core/Src/fsm_reprocesadora1.o \
./Core/Src/global_variables.o \
./Core/Src/i2c_pressure.o \
./Core/Src/main.o \
./Core/Src/nextion.o \
./Core/Src/ntc_adc.o \
./Core/Src/onewire.o \
./Core/Src/relay_module.o \
./Core/Src/rfid_wiegand.o \
./Core/Src/rs485_pressure_sensor.o \
./Core/Src/service.o \
./Core/Src/softserial.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/ticket.o \
./Core/Src/user_gpio.o \
./Core/Src/watchdog.o 

C_DEPS += \
./Core/Src/CSN_A2.d \
./Core/Src/ENDOCLEAN2000.d \
./Core/Src/MAX6675.d \
./Core/Src/Medisafe.d \
./Core/Src/Medisafe_Gpio_Config.d \
./Core/Src/Medisafe_Mode_Config.d \
./Core/Src/SystemTicks.d \
./Core/Src/UltrasonicGenerator.d \
./Core/Src/ds18b20.d \
./Core/Src/fsm_reprocesadora1.d \
./Core/Src/global_variables.d \
./Core/Src/i2c_pressure.d \
./Core/Src/main.d \
./Core/Src/nextion.d \
./Core/Src/ntc_adc.d \
./Core/Src/onewire.d \
./Core/Src/relay_module.d \
./Core/Src/rfid_wiegand.d \
./Core/Src/rs485_pressure_sensor.d \
./Core/Src/service.d \
./Core/Src/softserial.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/ticket.d \
./Core/Src/user_gpio.d \
./Core/Src/watchdog.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Oz -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/CSN_A2.cyclo ./Core/Src/CSN_A2.d ./Core/Src/CSN_A2.o ./Core/Src/CSN_A2.su ./Core/Src/ENDOCLEAN2000.cyclo ./Core/Src/ENDOCLEAN2000.d ./Core/Src/ENDOCLEAN2000.o ./Core/Src/ENDOCLEAN2000.su ./Core/Src/MAX6675.cyclo ./Core/Src/MAX6675.d ./Core/Src/MAX6675.o ./Core/Src/MAX6675.su ./Core/Src/Medisafe.cyclo ./Core/Src/Medisafe.d ./Core/Src/Medisafe.o ./Core/Src/Medisafe.su ./Core/Src/Medisafe_Gpio_Config.cyclo ./Core/Src/Medisafe_Gpio_Config.d ./Core/Src/Medisafe_Gpio_Config.o ./Core/Src/Medisafe_Gpio_Config.su ./Core/Src/Medisafe_Mode_Config.cyclo ./Core/Src/Medisafe_Mode_Config.d ./Core/Src/Medisafe_Mode_Config.o ./Core/Src/Medisafe_Mode_Config.su ./Core/Src/SystemTicks.cyclo ./Core/Src/SystemTicks.d ./Core/Src/SystemTicks.o ./Core/Src/SystemTicks.su ./Core/Src/UltrasonicGenerator.cyclo ./Core/Src/UltrasonicGenerator.d ./Core/Src/UltrasonicGenerator.o ./Core/Src/UltrasonicGenerator.su ./Core/Src/ds18b20.cyclo ./Core/Src/ds18b20.d ./Core/Src/ds18b20.o ./Core/Src/ds18b20.su ./Core/Src/fsm_reprocesadora1.cyclo ./Core/Src/fsm_reprocesadora1.d ./Core/Src/fsm_reprocesadora1.o ./Core/Src/fsm_reprocesadora1.su ./Core/Src/global_variables.cyclo ./Core/Src/global_variables.d ./Core/Src/global_variables.o ./Core/Src/global_variables.su ./Core/Src/i2c_pressure.cyclo ./Core/Src/i2c_pressure.d ./Core/Src/i2c_pressure.o ./Core/Src/i2c_pressure.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/nextion.cyclo ./Core/Src/nextion.d ./Core/Src/nextion.o ./Core/Src/nextion.su ./Core/Src/ntc_adc.cyclo ./Core/Src/ntc_adc.d ./Core/Src/ntc_adc.o ./Core/Src/ntc_adc.su ./Core/Src/onewire.cyclo ./Core/Src/onewire.d ./Core/Src/onewire.o ./Core/Src/onewire.su ./Core/Src/relay_module.cyclo ./Core/Src/relay_module.d ./Core/Src/relay_module.o ./Core/Src/relay_module.su ./Core/Src/rfid_wiegand.cyclo ./Core/Src/rfid_wiegand.d ./Core/Src/rfid_wiegand.o ./Core/Src/rfid_wiegand.su ./Core/Src/rs485_pressure_sensor.cyclo ./Core/Src/rs485_pressure_sensor.d ./Core/Src/rs485_pressure_sensor.o ./Core/Src/rs485_pressure_sensor.su ./Core/Src/service.cyclo ./Core/Src/service.d ./Core/Src/service.o ./Core/Src/service.su ./Core/Src/softserial.cyclo ./Core/Src/softserial.d ./Core/Src/softserial.o ./Core/Src/softserial.su ./Core/Src/stm32f1xx_hal_msp.cyclo ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.cyclo ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.cyclo ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/ticket.cyclo ./Core/Src/ticket.d ./Core/Src/ticket.o ./Core/Src/ticket.su ./Core/Src/user_gpio.cyclo ./Core/Src/user_gpio.d ./Core/Src/user_gpio.o ./Core/Src/user_gpio.su ./Core/Src/watchdog.cyclo ./Core/Src/watchdog.d ./Core/Src/watchdog.o ./Core/Src/watchdog.su

.PHONY: clean-Core-2f-Src

