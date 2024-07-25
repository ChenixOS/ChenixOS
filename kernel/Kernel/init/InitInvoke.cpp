#include "InitInvoke.h"

#include "types.h"

#include "klib/stdio.h"

#include "init/Init.h"

extern "C" int KINIT_ARRAY_START;
extern "C" int KINIT_ARRAY_END;

static const char* g_StageNames[] = {
    nullptr,
    "DevDrivers",
    "FsDrivers",
    "ExecHandlers",
    "DelayHandlers",
    "BusDrivers",
    "BusScan"
};

void CallInitFuncs(int stage) {
    if(stage != INIT_STAGE_DELAY) {
        klog_info("Init", "Calling init funcs for stage %s", g_StageNames[stage]);
    }

    uint64* arr = (uint64*)&KINIT_ARRAY_START;
    uint64* arrEnd = (uint64*)&KINIT_ARRAY_END;
    for(; arr != arrEnd; arr += 2) {
        void (*func)() = (void(*)())arr[0];
        uint64 s = arr[1];
        if(stage == s) {
            func();
        }
    }
}