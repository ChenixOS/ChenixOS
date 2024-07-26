#pragma once

#include "stdint.h"
#include "types.h"

#include <vector>

// EAX EBX ECX EDX (4 x 4)
#define MAX_CPUID_OUTPUT_LENGTH (4 * 4) + 1

struct CPUInfo {
    uint64 apicId;
    char vendor[MAX_CPUID_OUTPUT_LENGTH];
    char model[MAX_CPUID_OUTPUT_LENGTH];
};

extern std::vector<CPUInfo> g_CpuInfoList;
extern void InitCpuInfo();