cpp_compiler := $(ELF_GCC)
asm_compiler := 

libs := $(bin_dir)/libsimpleos2/libsimpleos2.a

int_dir := $(int_dir)/Programs/Login
bin_dir := $(bin_dir)/Programs/Login

includes := $(root_dir)/libs/libsimpleos2

compile_flags := -g -ffreestanding -fno-stack-protector -fno-exceptions -fno-rtti
link_flags := -g -nostdlib -e __start -static -lgcc

out_file := login.elf


include $(build_dir)/makefile/cpp.mk