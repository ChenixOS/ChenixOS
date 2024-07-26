cpp_compiler := $(ELF_GCC)
asm_compiler := 

libs := $(bin_dir)/libc/libc.a

int_dir := $(int_dir)/Programs/ctest
bin_dir := $(bin_dir)/Programs/ctest

includes := $(root_dir)/libs/libc/include

compile_flags := -g -ffreestanding -fno-stack-protector -fno-exceptions
link_flags := -g -nostdlib -e __start -static -lgcc

out_file := ctest.elf


include $(build_dir)/makefile/cpp.mk