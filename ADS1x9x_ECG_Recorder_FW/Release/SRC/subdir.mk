################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SRC/ADS1x9x.c \
../SRC/ADS1x9x_ECG_Processing.c \
../SRC/ADS1x9x_Nand_Flash.c \
../SRC/ADS1x9x_Nand_LLD.c \
../SRC/ADS1x9x_RESP_Processing.c \
../SRC/ADS1x9x_USB_Communication.c \
../SRC/ADS1x9x_main.c \
../SRC/USB_constructs.c 

OBJS += \
./SRC/ADS1x9x.obj \
./SRC/ADS1x9x_ECG_Processing.obj \
./SRC/ADS1x9x_Nand_Flash.obj \
./SRC/ADS1x9x_Nand_LLD.obj \
./SRC/ADS1x9x_RESP_Processing.obj \
./SRC/ADS1x9x_USB_Communication.obj \
./SRC/ADS1x9x_main.obj \
./SRC/USB_constructs.obj 

C_DEPS += \
./SRC/ADS1x9x.pp \
./SRC/ADS1x9x_ECG_Processing.pp \
./SRC/ADS1x9x_Nand_Flash.pp \
./SRC/ADS1x9x_Nand_LLD.pp \
./SRC/ADS1x9x_RESP_Processing.pp \
./SRC/ADS1x9x_USB_Communication.pp \
./SRC/ADS1x9x_main.pp \
./SRC/USB_constructs.pp 

C_SRCS_QUOTED += \
"../SRC/ADS1x9x.c" \
"../SRC/ADS1x9x_ECG_Processing.c" \
"../SRC/ADS1x9x_Nand_Flash.c" \
"../SRC/ADS1x9x_Nand_LLD.c" \
"../SRC/ADS1x9x_RESP_Processing.c" \
"../SRC/ADS1x9x_USB_Communication.c" \
"../SRC/ADS1x9x_main.c" \
"../SRC/USB_constructs.c" 


# Each subdirectory must supply rules for building sources it contributes
SRC/ADS1x9x.obj: ../SRC/ADS1x9x.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="SRC/ADS1x9x.pp" --obj_directory="SRC" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SRC/ADS1x9x_ECG_Processing.obj: ../SRC/ADS1x9x_ECG_Processing.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="SRC/ADS1x9x_ECG_Processing.pp" --obj_directory="SRC" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SRC/ADS1x9x_Nand_Flash.obj: ../SRC/ADS1x9x_Nand_Flash.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="SRC/ADS1x9x_Nand_Flash.pp" --obj_directory="SRC" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SRC/ADS1x9x_Nand_LLD.obj: ../SRC/ADS1x9x_Nand_LLD.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="SRC/ADS1x9x_Nand_LLD.pp" --obj_directory="SRC" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SRC/ADS1x9x_RESP_Processing.obj: ../SRC/ADS1x9x_RESP_Processing.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="SRC/ADS1x9x_RESP_Processing.pp" --obj_directory="SRC" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SRC/ADS1x9x_USB_Communication.obj: ../SRC/ADS1x9x_USB_Communication.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="SRC/ADS1x9x_USB_Communication.pp" --obj_directory="SRC" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SRC/ADS1x9x_main.obj: ../SRC/ADS1x9x_main.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="SRC/ADS1x9x_main.pp" --obj_directory="SRC" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SRC/USB_constructs.obj: ../SRC/USB_constructs.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="SRC/USB_constructs.pp" --obj_directory="SRC" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '


