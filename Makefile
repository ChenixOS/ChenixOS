# ===========================================
#        ChenixOS`s Build Tools
# ===========================================

# currently, only Debug and Release have special meaning
# the configuration is best changed by invoking make with "config=Release"
export CONFIG ?= Debug
export ARCH ?= x86_64

export OS_NAME := ChenixOS
export VERSION := 1.0
export BUILD_DATE := $(shell date -R)

# Build enviroment
export PE_GCC := x86_64-w64-mingw32-gcc
export ELF_GCC := gcc
export NASM := nasm
export MTOOLS_MMD := mmd
export MTOOLS_MCOPY := mcopy
export DEBUGFS := debugfs
export PARTED := parted
export MKFS_FAT := mkfs.fat

# This is the OS-src root directory
export root_dir := $(shell pwd)
# This is the main build directory, it should only be changed after "make clean" was called
export build_dir := $(root_dir)/build
# OUT-Directory
export out_dir := $(build_dir)/out/$(CONFIG)
# This is the directory in which all binary files will be stored (should be inside the build_dir folder)
export bin_dir := $(build_dir)/out/$(CONFIG)/bin
# This is the directory in which all intermediate files will be stored (should be inside the build_dir folder)
export int_dir := $(build_dir)/out/$(CONFIG)/int
# 引用库目录
export dep_dir := $(root_dir)/dep


# Default: build common boot disk image.
defualt: disk vbox_img qcow2_img finish
all: disk vbox_img vhd_img vmdk_img qcow_img qcow2_img finish

# 启动QEMU
runqemu: FORCE
	@ printf "\e[32mStarting SimpleOS2 in Qemu (No-KVM)\e[0m\n"
	qemu-system-x86_64 -gdb tcp::1234 \
		-machine q35 \
		-m 1024 \
		-cpu qemu64 \
		-smp 1 \
		-net none \
		-drive if=pflash,unit=0,format=raw,file=dep/ovmf/x64/OVMF_CODE.fd,readonly=on \
		-drive if=pflash,unit=1,format=raw,file=dep/ovmf/x64/OVMF_VARS.fd,readonly=on \
		-drive if=ide,file=$(out_dir)/SimpleOS2.img,format=raw

# 启动QEMU-KVM
runkvm: FORCE
	@ printf "\e[32mStarting SimpleOS2 in Qemu (KVM)\e[0m\n"
	qemu-system-x86_64 -enable-kvm \
		-machine q35 \
		-m 1024 \
		-cpu host,+invtsc \
		-smp 1 \
		-net none \
		-drive if=pflash,unit=0,format=raw,file=dep/ovmf/x64/OVMF_CODE.fd,readonly=on \
		-drive if=pflash,unit=1,format=raw,file=dep/ovmf/x64/OVMF_VARS.fd,readonly=on \
		-drive file=$(out_dir)/SimpleOS2.img,format=raw

# Build For build-time tools
rdbuilder: FORCE
	@ printf "\e[32mBuilding initrd tool\e[0m\n"
	@ $(MAKE) -s -C build/RamdiskBuilder

# Build For kernel and kernel external
bootloader: FORCE
	@ printf "\e[32mBuilding Bootloader\e[0m\n"
	@ $(MAKE) -s -C kernel/Bootloader
kernel: FORCE
	@ $(MAKE) -s acpica
	@ $(MAKE) -s dep
	@ printf "\e[32mBuilding Kernel\e[0m\n"
	@ $(MAKE) -s -C kernel/Kernel
# Build Dep Library
dep: FORCE
	@ printf "\e[32mBuilding Dep Library\e[0m\n"
acpica: FORCE
	@ printf "\e[32mBuilding ACPICA\e[0m\n"
	@ $(MAKE) -s -C dep/acpica

# Build for userland base library
libs: FORCE
	@ $(MAKE) -s libsimpleos2
	@ $(MAKE) -s libc
libsimpleos2: FORCE
	@ printf "\e[32mBuilding LibSimpleOS2\e[0m\n"
	@ $(MAKE) -s -C libs/libsimpleos2
libc: FORCE
	@ printf "\e[32mBuilding LibC\e[0m\n"
	@ $(MAKE) -s -C libs/libc

# Build for something userland program
programs: FORCE
	@ printf "\e[32mBuilding Init\e[0m\n"
	@ $(MAKE) -s -C userland/Programs/Init
	@ printf "\e[32mBuilding Shell\e[0m\n"
	@ $(MAKE) -s -C userland/Programs/Shell
	@ printf "\e[32mBuilding TestVFS\e[0m\n"
	@ $(MAKE) -s -C userland/Programs/TestVFS
	@ printf "\e[32mBuilding Echo\e[0m\n"
	@ $(MAKE) -s -C userland/Programs/Echo
	@ printf "\e[32mBuilding Cat\e[0m\n"
	@ $(MAKE) -s -C userland/Programs/Cat
	@ printf "\e[32mBuilding TestThreading\e[0m\n"
	@ $(MAKE) -s -C userland/Programs/TestThreading
	@ printf "\e[32mBuilding Login\e[0m\n"
	@ $(MAKE) -s -C userland/Programs/Login
	@ printf "\e[32mBuilding LibcTest\e[0m\n"
	@ $(MAKE) -s -C userland/Programs/libctest

# Build for initrd.img
initrd: FORCE
	@ $(MAKE) -s rdbuilder
	@ $(MAKE) -s libs
	@ $(MAKE) -s programs
	@ printf "\e[32mBuilding initrd\e[0m\n"
	@ $(MAKE) -s -f build/kernel/initrd.mk

# Build for OS`s Disk Image Or Boot Image
partition: FORCE
	@ $(MAKE) -s bootloader
	@ $(MAKE) -s kernel
	@ $(MAKE) -s initrd
	@ printf "\e[32mBuilding SimpleOS2 boot partition\e[0m\n"
	@ $(MAKE) -s -f build/kernel/partition.mk

disk: FORCE
	@ $(MAKE) -s partition
	@ printf "\e[32mBuilding SimpleOS2 raw disk image\e[0m\n"
	@ $(MAKE) -s -f build/kernel/disk.mk
vbox_img: FORCE disk
	@ printf "\e[32mBuilding SimpleOS2 VirtualBox disk image\e[0m\n"
	@ qemu-img convert -f raw -O vdi $(out_dir)/SimpleOS2.img $(out_dir)/SimpleOS2.vdi -p
vmdk_img: FORCE disk
	@ printf "\e[32mBuilding SimpleOS2 VMWare disk image\e[0m\n"
	@ qemu-img convert -f raw -O vmdk $(out_dir)/SimpleOS2.img $(out_dir)/SimpleOS2.vmdk -p
vhd_img: FORCE disk
	@ printf "\e[32mBuilding SimpleOS2 Hyper-V disk image\e[0m\n"
	@ qemu-img convert -f raw -O vpc $(out_dir)/SimpleOS2.img $(out_dir)/SimpleOS2.vhd -p
qcow_img: FORCE disk
	@ printf "\e[32mBuilding SimpleOS2 QEMU-COW disk image\e[0m\n"
	@ qemu-img convert -f raw -O qcow $(out_dir)/SimpleOS2.img $(out_dir)/SimpleOS2.qcow -p
qcow2_img: FORCE disk
	@ printf "\e[32mBuilding SimpleOS2 QEMU-COW2 disk image\e[0m\n"
	@ qemu-img convert -f raw -O qcow2 $(out_dir)/SimpleOS2.img $(out_dir)/SimpleOS2.qcow2 -p

# Clean for project cache
clean: FORCE
	@ printf "\e[32mCleaning up\e[0m\n"
	@ rm -rf $(build_dir)/out

finish: FORCE
	@ printf "\e[32mBuild finish.\e[0m\n"
	@ printf "Out Image: $(out_dir)/SimpleOS2.img \n"

FORCE: 
