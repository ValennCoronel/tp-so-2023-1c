################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif




BUILD_ARTIFACT_NAME := kernel
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: kernel

dependents:
	-cd /home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy && $(MAKE) all

# Tool invocations
kernel: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" -o "kernel" $(OBJS) $(USER_OBJS) $(LIBS) -lcommons -lglobal
	@echo 'Finished building target: $@'
	@echo ' '

valgrind: main-build
	valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./kernel

# Other Targets
clean:
	-$(RM) kernel
	-$(RM) valgrind.log
	-$(RM) kernel.log
	-@echo ' '

.PHONY: all clean dependents main-build

