## Chenix OS

*** 中文  |  [English](README-en.md) ***

[![测试镜像CI](https://github.com/ChenixOS/ChenixOS/actions/workflows/test-image.yml/badge.svg)](https://github.com/ChenixOS/ChenixOS/actions/workflows/test-image.yml)


### 简介

ChenixOS是基于X86-64位架构开发的64位类Unix操作系统，目的是提供一个轻量级、完全可控、可定制的内核以及相关的驱动、用户程序的合集。与Linux/BSD系列不同的是，Chenix OS采用C++面向对象设计的内核。

- 构建环境：Ubuntu
- 构建工具：GCC、Mingw-GCC、NASM、mkfs.fat、mkfs.ext2、MTOOLS、QEMU
- 开发环境：VSCode
- 开源协议：Apache V2 (额外条款,主要是关于版权和专利授权的)

### 各目录的用途

`build`     编译和构建需要用到的文件或者程序

`dep`       引用的相关头文件和库

`docs`      相关的开发文档以及说明文件

`kernel`    内核及相关内置驱动的源代码目录

`libs`      提供给用户进行开发使用和编译使用的库

`userland`  用户层的相关程序

### 构建方法
- 配置Ubuntu环境（可以执行build/toolchain/install-from-apt.sh）
- 打开命令行，切换到根目录
- 执行`make`或者`make all`
- 把out目录下的测试镜像丢到虚拟机中运行 (也可以直接`make runqemu`)

### 计划表

以下是列入了计划内要实现的功能，同时欢迎大家提Issue和PR！

- 中文Issue请提交到：https://gitee.com/chenixos/chenixos
- 英文Issue请提交到：https://github.com/ChenixOS/ChenixOS
- 会中文却非要提交英文的请到：file:///dev/null


#### 已支持功能

- EFI/UEFI 64位模式启动
- 与Linux/FreeBSD类似的VFS
- SMP模式的多CPU核心支持
- 多任务/多线程
- 内置对ACPI、PIC、APIC、I/O APIC、PCI/PCIe的驱动实现
- 大部分的POSIX/LibC支持


#### 实验性功能(未稳定)
- AHCI基本可用（在QEMU和VBox上可用，VMWare上状态不正常）


#### 正在做：
- 完善procfs
- 引入对GPT分区和虚拟磁盘的支持

#### 计划做:
- 引入Kconfig
- 支持抢占式多任务
- 添加IDE/SATA硬盘支持 (SATA已完成)
- 尝试兼容Linux的syscall



