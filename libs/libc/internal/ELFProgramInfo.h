#pragma once

#include "internal_types.h"


struct ELFProgramInfo;

struct ELFThread {
    ELFThread* selfPtr;
    void* allocPtr;
    ELFProgramInfo* progInfo;
};

struct ELFProgramInfo {
    char* masterTLSAddress;
    uint64 masterTLSSize;
    int argc;
    char** argv;
    char** environ;
};