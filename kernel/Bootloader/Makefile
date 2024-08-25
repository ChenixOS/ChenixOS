cpp_compiler := $(PE_GCC)
asm_compiler := 

int_dir := $(int_dir)/Bootloader
bin_dir := $(bin_dir)/Bootloader

includes := $(root_dir)/dep/gnu-efi/inc $(root_dir)/dep/gnu-efi/inc/x86_64 $(root_dir)/kernel/Kernel/include/global

compile_flags := -ffreestanding
link_flags := -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main

ifeq ($(CONFIG),Release)
	compile_flags += -O2
endif

out_file := BOOTX64.EFI

include $(build_dir)/makefile/cpp.mk