cpp_compiler := $(ELF_GCC)
asm_compiler := 

int_dir := $(int_dir)/acpica
bin_dir := $(bin_dir)/acpica

includes := include/acpica

compile_flags := -ffreestanding -fno-stack-protector -fno-exceptions -DACPI_LIBRARY -fPIC -O1

out_file := acpica.a

include $(build_dir)/makeinclude/cpp_static.mk