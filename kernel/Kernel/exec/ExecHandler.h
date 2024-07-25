#pragma once

#include "types.h"
#include "interrupts/IDT.h"
#include "klib/AnchorList.h"

class ExecHandler {
public:
    virtual bool CheckAndPrepare(uint8* buffer, uint64 bufferSize, uint64 pml4Entry, IDT::Registers* regs, int argc, const char* const* argv,int envCount, const char* const* env) = 0;

public:
    ktl::Anchor<ExecHandler> m_Anchor;
};

class ExecHandlerRegistry {
public:
    static void RegisterHandler(ExecHandler* handler);
    static void UnregisterHandler(ExecHandler* handler);

    static bool Prepare(uint8* buffer, uint64 bufferSize, uint64 pml4Entry, IDT::Registers* regs, int argc, const char* const* argv);
    static bool Prepare(uint8* buffer, uint64 bufferSize, uint64 pml4Entry, IDT::Registers* regs, int argc, const char* const* argv, int envCount,const char* const* env);
};