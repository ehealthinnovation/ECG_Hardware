################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../lnk_msp430f5529.cmd 


# Each subdirectory must supply rules for building sources it contributes
lnk_msp430f5529.out: ../lnk_msp430f5529.cmd $(OBJS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Linker'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" -z -m"ADS1292R_ECG_Recorder.map" --stack_size=160 --heap_size=160 --use_hw_mpy=F5 --warn_sections -i"C:/Program Files/Texas Instruments/ccsv4/msp430/include" -i"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/lib" -i"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --reread_libs --rom_model -o "$@" "$<" "../lnk_msp430f5529.cmd" $(ORDERED_OBJS)
	@echo 'Finished building: $<'
	@echo ' '


