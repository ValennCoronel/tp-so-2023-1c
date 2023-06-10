################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/utils/utils_cliente.c \
../src/utils/utils_server.c 

C_DEPS += \
./src/utils/utils_cliente.d \
./src/utils/utils_server.d 

OBJS += \
./src/utils/utils_cliente.o \
./src/utils/utils_server.o 


# Each subdirectory must supply rules for building sources it contributes
src/utils/%.o: ../src/utils/%.c src/utils/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-utils

clean-src-2f-utils:
	-$(RM) ./src/utils/utils_cliente.d ./src/utils/utils_cliente.o ./src/utils/utils_server.d ./src/utils/utils_server.o

.PHONY: clean-src-2f-utils

