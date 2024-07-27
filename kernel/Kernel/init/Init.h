#pragma once

// 用于初始化一些仅使用标准IO或者ACPI进行处理的驱动，或者把设备附加到总线的处理列表
#define INIT_STAGE_DEVDRIVERS 1
// 用于注册文件系统
#define INIT_STAGE_FSDRIVERS 2
// 用于注册ExecHandlers
#define INIT_STAGE_EXECHANDLERS 3
// 最后用于延迟处理的函数
#define INIT_STAGE_DELAY 4
// 用于注册需要依赖总线或者需要延迟加载的设备驱动，或者用于总线驱动的预先扫描
#define INIT_STAGE_BUSDRIVERS 5
// 用于扫描总线设备的函数（不推荐一般设备去使用，仅用于PCI或者扫描其他总线的`总线驱动`）
// 既当总线初始化成功后，需要在后面把相应的设备信息通知到其他驱动时使用
#define INIT_STAGE_BUSPROBE 6

#define REGISTER_INIT_FUNC(func, stage) static uint64 func##_kinit_array_entry[] __attribute__((used)) __attribute__((section(".kinit_array"))) = { (uint64)&func, stage }