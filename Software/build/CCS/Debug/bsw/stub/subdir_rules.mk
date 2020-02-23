################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
bsw/stub/stub.obj: R:/MyAutosar/MemoryStack/Software/bsw/stub/stub.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="R:/MyAutosar/MemoryStack/Software/build/CCS" --include_path="R:/MyAutosar/MemoryStack/Software/bsw/static/Mcal/Eep/inc" --include_path="R:/MyAutosar/MemoryStack/Software/bsw/static/Infrastructure/inc" --include_path="R:/MyAutosar/MemoryStack/Software/bsw/static/Interface/EA/inc" --include_path="R:/MyAutosar/MemoryStack/Software/bsw/gen" --include_path="R:/MyAutosar/MemoryStack/Software/bsw/static/Infrastructure/platform/inc" --include_path="R:/MyAutosar/MemoryStack/Software/bsw/stub" --include_path="C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="bsw/stub/$(basename $(<F)).d_raw" --obj_directory="bsw/stub" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


