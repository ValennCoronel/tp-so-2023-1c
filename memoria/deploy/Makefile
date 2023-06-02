################################################################################
# Automatically-generated file. Do not edit!
################################################################################


RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include obj/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif


BUILD_ARTIFACT_NAME := memoria
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: memoria

# Tool invocations
memoria: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "memoria" $(OBJS) $(USER_OBJS) $(LIBS) -lcommons -lpthread
	@echo 'Finished building target: $@'
	@echo ' '

valgrind: main-build
	valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./memoria

# Other Targets
clean:
	-$(RM) memoria
	-$(RM) valgrind.log
	-$(RM) memoria.log
	-@echo ' '

.PHONY: all clean dependents main-build
