################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_Common/UsbIsr.c \
../USB_Common/descriptors.c \
../USB_Common/dma.c \
../USB_Common/usb.c \
../USB_Common/usb_eventHandling.c 

OBJS += \
./USB_Common/UsbIsr.obj \
./USB_Common/descriptors.obj \
./USB_Common/dma.obj \
./USB_Common/usb.obj \
./USB_Common/usb_eventHandling.obj 

C_DEPS += \
./USB_Common/UsbIsr.pp \
./USB_Common/descriptors.pp \
./USB_Common/dma.pp \
./USB_Common/usb.pp \
./USB_Common/usb_eventHandling.pp 

C_SRCS_QUOTED += \
"../USB_Common/UsbIsr.c" \
"../USB_Common/descriptors.c" \
"../USB_Common/dma.c" \
"../USB_Common/usb.c" \
"../USB_Common/usb_eventHandling.c" 


# Each subdirectory must supply rules for building sources it contributes
USB_Common/UsbIsr.obj: ../USB_Common/UsbIsr.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_Common/UsbIsr.pp" --obj_directory="USB_Common" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

USB_Common/descriptors.obj: ../USB_Common/descriptors.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_Common/descriptors.pp" --obj_directory="USB_Common" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

USB_Common/dma.obj: ../USB_Common/dma.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_Common/dma.pp" --obj_directory="USB_Common" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

USB_Common/usb.obj: ../USB_Common/usb.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_Common/usb.pp" --obj_directory="USB_Common" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

USB_Common/usb_eventHandling.obj: ../USB_Common/usb_eventHandling.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_Common/usb_eventHandling.pp" --obj_directory="USB_Common" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '


