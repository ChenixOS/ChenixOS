#include "ExecHandler.h"

static ktl::AnchorList<ExecHandler, &ExecHandler::m_Anchor> g_Handlers;

void ExecHandlerRegistry::RegisterHandler(ExecHandler* handler) {
    g_Handlers.push_back(handler);
}
void ExecHandlerRegistry::UnregisterHandler(ExecHandler* handler) {
    g_Handlers.erase(handler);
}

bool ExecHandlerRegistry::Prepare(uint8* buffer, uint64 bufferSize, uint64 pml4Entry, IDT::Registers* regs, int argc, const char* const* argv) {
    return ExecHandlerRegistry::Prepare(buffer,bufferSize,pml4Entry,regs,argc,argv,0,new char*[1]);
}

bool ExecHandlerRegistry::Prepare(uint8* buffer, uint64 bufferSize, uint64 pml4Entry, IDT::Registers* regs, int argc, const char* const* argv,int envCount,const char* const* env) {
    for(ExecHandler& handler : g_Handlers) {
        if(handler.CheckAndPrepare(buffer, bufferSize, pml4Entry, regs, argc, argv,envCount,env))
            return true;
    }

    return false;
}