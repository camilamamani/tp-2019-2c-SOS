################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libSUSE.c \
../log_suse.c \
../protocol.c \
../socket.c \
../utils.c 

OBJS += \
./libSUSE.o \
./log_suse.o \
./protocol.o \
./socket.o \
./utils.o 

C_DEPS += \
./libSUSE.d \
./log_suse.d \
./protocol.d \
./socket.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


