################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_CDC_API/UsbCdc.c 

OBJS += \
./USB_CDC_API/UsbCdc.obj 

C_DEPS += \
./USB_CDC_API/UsbCdc.pp 

C_SRCS_QUOTED += \
"../USB_CDC_API/UsbCdc.c" 


# Each subdirectory must supply rules for building sources it contributes
USB_CDC_API/UsbCdc.obj: ../USB_CDC_API/UsbCdc.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_CDC_API/UsbCdc.pp" --obj_directory="USB_CDC_API" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '


