################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/filesystem.c \
../src/peticiones_kernel.c \


C_DEPS += \
./src/filesystem.d \
./src/peticiones_kernel.d \


OBJS += \
./src/filesystem.o \
./src/peticiones_kernel.o \

# Each subdirectory must supply rules for building sources it contributes
src/filesystem.o: ../src/filesystem.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/src" -Icommons -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/src" -Icommons -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/filesystem.d ./src/filesystem.o ./src/peticiones_kernel.d ./src/peticiones_kernel.o

.PHONY: clean-src

