################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/kernel.c \
../src/peticiones_cpu.c \
../src/peticiones_fs.c \
../src/planificador_corto_plazo.c \
../src/planificador_largo_plazo.c \

C_DEPS += \
./src/kernel.d \
./src/peticiones_cpu.d \
./src/peticiones_fs.d \
./src/planificador_corto_plazo.d \
./src/planificador_largo_plazo.d \

OBJS += \
./src/kernel.o \
./src/peticiones_cpu.o \
./src/peticiones_fs.o \
./src/planificador_corto_plazo.o \
./src/planificador_largo_plazo.o \


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/src" -Icommons -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/kernel.d ./src/kernel.o ./src/peticiones_cpu.d ./src/peticiones_cpu.o ./src/peticiones_fs.d ./src/peticiones_fs.o ./src/planificador_corto_plazo.d ./src/planificador_corto_plazo.o ./src/planificador_largo_plazo.d ./src/planificador_largo_plazo.o
	
.PHONY: clean-src

