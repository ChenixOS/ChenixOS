#pragma once

#include <stdint.h>

struct __attribute__((packed)) SymbolItem
{
    char name[64];
    void* ptr;
};

#define EXPORT_SYMBOL(name,func) static struct SymbolItem func##_symbol_table_entry __attribute__((used)) __attribute__((section(".symbol_table"))) = { name, (void*)&func }
#define EXPORT_CPP_SYMBOL(name,iname,func) static struct SymbolItem iname##_symbol_table_entry __attribute__((used)) __attribute__((section(".symbol_table"))) = { name, (void*)&func }

namespace SymbolTable {
    const SymbolItem* findItem(const char* name);
}

