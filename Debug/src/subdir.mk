################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CU_SRCS += \
../src/cudaFuncs.cu 

C_SRCS += \
../src/parallel_project.c \
../src/sequential.c \
../src/stringFuncs.c 

O_SRCS += \
../src/cudaFuncs.o \
../src/parallel_project.o \
../src/sequential.o \
../src/stringFuncs.o 

OBJS += \
./src/cudaFuncs.o \
./src/parallel_project.o \
./src/sequential.o \
./src/stringFuncs.o 

CU_DEPS += \
./src/cudaFuncs.d 

C_DEPS += \
./src/parallel_project.d \
./src/sequential.d \
./src/stringFuncs.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cu
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	mpicc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	mpicc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


