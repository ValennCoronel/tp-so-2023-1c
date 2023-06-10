################################################################################
# Automatically-generated file. Do not edit!
################################################################################


RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif


BUILD_ARTIFACT_NAME := consola
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: consola

# Tool invocations
consola: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" -o "consola" $(OBJS) $(USER_OBJS) $(LIBS) -lcommons -lglobal -lpthread
	@echo 'Finished building target: $@'
	@echo ' '

valgrind: main-build
	valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./consola

# Other Targets
clean:
	-$(RM) consola
	-$(RM) valgrind.log
	-$(RM) consola.log
	-@echo ' '

.PHONY: all clean dependents main-build

-include ../makefile.targets
