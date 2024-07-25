cpp_compiler := $(ELF_GCC)
asm_compiler := 

libs := $(bin_dir)/libsimpleos2/libsimpleos2.a

int_dir := $(int_dir)/Programs/Init
bin_dir := $(bin_dir)/Programs/Init

includes := $(root_dir)/libs/libsimpleos2

compile_flags := -g -ffreestanding -fno-stack-protector -fno-exceptions -m64
link_flags := -g -nostdlib -e __start -static -lgcc

out_file := init.elf


include $(build_dir)/makeinclude/cpp.mk