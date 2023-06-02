################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/kernel.c \
../src/mock_envio_cpu.c \
../src/peticiones_cpu.c \
../src/planificador_corto_plazo.c \
../src/planificador_largo_plazo.c \
../src/utils_cliente.c \
../src/utils_server.c 

C_DEPS += \
./src/kernel.d \
./src/mock_envio_cpu.d \
./src/peticiones_cpu.d \
./src/planificador_corto_plazo.d \
./src/planificador_largo_plazo.d \
./src/utils_cliente.d \
./src/utils_server.d 

OBJS += \
./src/kernel.o \
./src/mock_envio_cpu.o \
./src/peticiones_cpu.o \
./src/planificador_corto_plazo.o \
./src/planificador_largo_plazo.o \
./src/utils_cliente.o \
./src/utils_server.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Icommons -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/kernel.d ./src/kernel.o ./src/mock_envio_cpu.d ./src/mock_envio_cpu.o ./src/peticiones_cpu.d ./src/peticiones_cpu.o ./src/planificador_corto_plazo.d ./src/planificador_corto_plazo.o ./src/planificador_largo_plazo.d ./src/planificador_largo_plazo.o ./src/utils_cliente.d ./src/utils_cliente.o ./src/utils_server.d ./src/utils_server.o

.PHONY: clean-src

