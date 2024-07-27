## Chenix OS

*** [中文](README.md)  |  English ***

[![Kernel Test Image CI](https://github.com/ChenixOS/ChenixOS/actions/workflows/test-image.yml/badge.svg)](https://github.com/ChenixOS/ChenixOS/actions/workflows/test-image.yml)

### Introduction

ChenixOS is a 64 bit Unix like operating system developed based on the X86-64 bit architecture, with the aim of providing a lightweight, fully controllable, customizable kernel and a collection of related drivers and user programs. Unlike the Linux/BSD series, Chenix OS adopts a C++object-oriented design kernel.

- Built environment: Ubuntu
- Building tools: GCC Mingw-GCC、NASM、mkfs.fat、mkfs.ext2、MTOOLS、QEMU
- Development environment: VSCode
- Open source license: Apache V2 (additional terms mainly regarding copyright and patent licensing)

### Get Started
- Configure Ubuntu environment (can execute build/toolchain/install-from-apt.sh)
- Open the command line and switch to the root directory
- Execute 'make' or 'make all'`
- Throw the test image from the out directory into the virtual machine and run it

### Schedule
The following are the planned features to be implemented, and we welcome everyone to raise issues and PRs!

- Please submit Chinese issues to: https://gitee.com/chenixos/chenixos
- Please submit the English issue to: https://github.com/ChenixOS/ChenixOS

#### Supported

- EFI/UEFI 64 bit mode boot
- VFS similar to Linux/FreeBSD
- Multi CPU core support in SMP mode
- Multi tasking/multi threading
- Built in driver implementation for API, PIC, APIC, I/O APIC, PCI/PCIe
- Most POSIX/LibC support

#### Experimental (unstable)
- AHCI is basically available (available on QEMU and VBox, abnormal status on VMWare)

#### Now Doing:
- Improve Procfs
- Introduce support for GPT partitions and virtual disks

#### Plan TODO:
- Introducing Kconfig
- Support preemptive multitasking
- Add IDE/SATA hard drive support (SATA completed)
- Attempt to be compatible with Linux's syscall
