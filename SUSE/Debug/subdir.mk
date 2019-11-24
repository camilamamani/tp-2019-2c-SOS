################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SUSE.c \
../configuracion.c \
../log.c \
../protocol.c \
../semaforos.c \
../servidor.c \
../socket.c \
../suse_ops.c \
../utils.c 

OBJS += \
./SUSE.o \
./configuracion.o \
./log.o \
./protocol.o \
./semaforos.o \
./servidor.o \
./socket.o \
./suse_ops.o \
./utils.o 

C_DEPS += \
./SUSE.d \
./configuracion.d \
./log.d \
./protocol.d \
./semaforos.d \
./servidor.d \
./socket.d \
./suse_ops.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


