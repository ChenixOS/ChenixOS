include $(build_dir)/makeinclude/base.mk

# 定义编译目录
cpp_compiler := $(ELF_GCC)
asm_compiler := $(NASM) -g -f elf64

libs := $(bin_dir)/acpica/acpica.a

int_dir := $(int_dir)/Kernel
bin_dir := $(bin_dir)/Kernel

includes := $(cur_dir)/include
includes += $(root_dir)/dep/acpica/include

# 定义参数

optimize_flags := -freg-struct-return -fno-bounds-check -finline-functions -finline-limit=1200 -fno-threadsafe-statics

compile_flags := -pie -fPIE -ffreestanding -fno-stack-protector -fno-exceptions -mno-sse -fno-rtti -fomit-frame-pointer -DACPI_LIBRARY -O2
link_flags := -pie -fPIE -nostdlib -Wl,--no-dynamic-linker -e main -lgcc -T linker.ld

ifeq ($(CONFIG),Debug)
	compile_flags += -g -DDEBUG -O1
	link_flags += -g -O1
endif
ifeq ($(CONFIG),ReleaseDbg)
	compile_flags += -g -DRELEASE -O2
	link_flags += -g -O2
endif
ifeq ($(CONFIG),Release)
	compile_flags += -DRELEASE -O2
	link_flags += -O2
endif

compile_flags += $(optimize_flags)

out_file := Kernel.sys

include $(build_dir)/makeinclude/cpp.mk