################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Common/hal_UCS.c \
../Common/hal_pmm.c \
../Common/hal_tlv.c 

OBJS += \
./Common/hal_UCS.obj \
./Common/hal_pmm.obj \
./Common/hal_tlv.obj 

C_DEPS += \
./Common/hal_UCS.pp \
./Common/hal_pmm.pp \
./Common/hal_tlv.pp 

C_SRCS_QUOTED += \
"../Common/hal_UCS.c" \
"../Common/hal_pmm.c" \
"../Common/hal_tlv.c" 


# Each subdirectory must supply rules for building sources it contributes
Common/hal_UCS.obj: ../Common/hal_UCS.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="Common/hal_UCS.pp" --obj_directory="Common" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

Common/hal_pmm.obj: ../Common/hal_pmm.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="Common/hal_pmm.pp" --obj_directory="Common" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

Common/hal_tlv.obj: ../Common/hal_tlv.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" --silicon_version=mspx -g -O2 --define=__MSP430F5529__ --define=__MSP430F5529 --include_path="C:/Program Files/Texas Instruments/ccsv4/msp430/include" --include_path="C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" --diag_warning=225 --sat_reassoc=off --fp_reassoc=off --plain_char=unsigned --printf_support=minimal --preproc_with_compile --preproc_dependency="Common/hal_tlv.pp" --obj_directory="Common" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '


