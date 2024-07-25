## Chenix OS

#### 简介

Chenix OS是基于X86-64位架构开发的64位类Unix操作系统，目的是提供一个轻量级、完全可控、可定制的内核以及相关的驱动、用户程序的合集。与Linux/BSD系列不同的是，Chenix OS采用C++面向对象设计的内核。

- 构建环境：Ubuntu
- 构建工具：GCC、Mingw-GCC、NASM、mkfs.fat、mkfs.ext2、MTOOLS、QEMU
- 开发环境：VSCode
- 开源协议：Apache V2 (以及额外条款)

#### 各目录的用途

`build`     编译和构建需要用到的文件或者程序
`dep`       引用的相关头文件和库
`docs`      相关的开发文档以及说明文件
`kernel`    内核及相关内置驱动的源代码目录
`libs`      提供给用户进行开发使用和编译使用的库
`userland`  用户层的相关程序


#### 已支持功能

- EFI/UEFI 64位模式启动
- 与Linux/FreeBSD类似的VFS
- SMP模式的多CPU核心支持
- 多任务/多线程
- 内置对ACPI、PIC、APIC、I/O APIC、PCI/PCIe的驱动实现
- 大部分的POSIX/LibC支持

#### TODO:

- 支持抢占式多任务
- 添加IDE/SATA硬盘支持
- 完善procfs
- 尝试兼容Linux的syscall

