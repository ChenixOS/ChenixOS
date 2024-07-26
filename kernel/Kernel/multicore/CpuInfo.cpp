#include "string.h"
#include "CpuInfo.h"
#include "arch/x64/x64.h"

#include "multicore/SMP.h"
#include "arch/APIC.h"

std::vector<CPUInfo> g_CpuInfoList;

void InitCpuInfo() {
    uint32 eax = 0,ebx = 0,ecx = 0,edx = 0;
    char buffer[MAX_CPUID_OUTPUT_LENGTH] = { 0 };
    CPUInfo info = { 0 };

    __cpuid(0, &eax, &ebx, &ecx, &edx);
    kmemcpy(buffer, &ebx, 4);
    kmemcpy(buffer + 4, &edx, 4);
    kmemcpy(buffer + 8, &ecx, 4);
    buffer[12] = '\0';

    kstrcpy(info.vendor, buffer);
    info.apicId = APIC::GetID();

    g_CpuInfoList.push_back(info);
}

// =======================================================================

#include "fs/procfs/ProcFs.h"
#include "init/Init.h"

void procfs_show_cpuinfo(ProcNode *node, VFS::Node *in, StringBuffer *buffer) 
{
    uint32 num = 0;
    char buf[256];

    for(auto &i : g_CpuInfoList) {
        buffer->append("processor\t: ");
        buffer->append(itoa(num++,buf,10));
        buffer->append("\n");

        buffer->append("vendor_id:\t: ");
        buffer->append(i.vendor);
        buffer->append("\n");

        buffer->append("apicid:\t: ");
        buffer->append(i.apicId);
        buffer->append("\n");

        // next line
        buffer->append("\n");
    }
}

static void MyInit() {
    procfs_new_string_callback_node("cpuinfo",procfs_show_cpuinfo);
}

REGISTER_INIT_FUNC(MyInit, INIT_STAGE_DELAY);