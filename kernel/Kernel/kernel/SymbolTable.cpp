#include "SymbolTable.h"
#include "string.h"

namespace SymbolTable {
    extern "C" int SYMBOL_TABLE_START;
    extern "C" int SYMBOL_TABLE_END;

    const SymbolItem* findItem(const char* name) {
        SymbolItem* arr = (SymbolItem*)&SYMBOL_TABLE_START;
        SymbolItem* arrEnd = (SymbolItem*)&SYMBOL_TABLE_END;

        for(; arr < arrEnd; arr++) {
            if(kstrcmp(arr->name, name) == 0) {
                return arr;
            }
        }

        return nullptr;
    }
}
