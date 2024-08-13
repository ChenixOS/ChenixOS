#pragma once

#include "types.h"

namespace Port
{
    /**
     * Writes a byte to the given port
     **/
    void OutByte(uint16 port, uint8 val);
    /**
     * Reads a byte from the given port
     **/
    uint8 InByte(uint16 port);

    void OutWord(uint16 port, uint16 val);
    uint16 InWord(uint16 port);

    void OutDWord(uint16 port, uint32 val);
    uint32 InDWord(uint16 port);
}

inline void outb(uint16 port,uint8 val) {
    Port::OutByte(port,val);
}

inline void outw(uint16 port,uint16 val) {
    Port::OutWord(port,val);
}

inline void outl(uint16 port,uint32 val) {
    Port::OutDWord(port,val);
}

inline uint8 inb(uint16 port) {
    return Port::InByte(port);
}

inline uint16 inw(uint16 port) {
    return Port::InWord(port);
}

inline uint32 inl(uint16 port) {
    return Port::InDWord(port);
}

// 批量写入

static inline void outsb(uint16 port, uint8 *data, uint32 count) {
    __asm__ __volatile__ (  
        "rep; outsb"  
        : "=D" (port), "=S" (data), "=c" (count)  
        : "0" (port), "1" (data), "2" (count)  
        : "memory"  
    );
}
static inline void outsw(uint16 port, uint16 *data, uint32 count) {
    __asm__ __volatile__ (  
        "rep; outsw"  
        : "=D" (port), "=S" (data), "=c" (count)  
        : "0" (port), "1" (data), "2" (count)  
        : "memory"  
    );
}
static inline void outsl(uint16 port, uint32 *data, uint32 count) {
    __asm__ __volatile__ (  
        "rep; outsl"  
        : "=D" (port), "=S" (data), "=c" (count)  
        : "0" (port), "1" (data), "2" (count)  
        : "memory"  
    );
}

// 批量读入

static inline void insb(uint16 port, uint8 *data, uint32 count) {
    __asm__ __volatile__ (  
        "rep; insb"  
        : "=S" (data), "=c" (count)
        : "d" (port), "0" (data), "1" (count)
        : "memory", "cc"
    );  
}
static inline void insw(uint16 port, uint16 *data, uint32 count) {
    __asm__ __volatile__ (  
        "rep; insw"  
        : "=S" (data), "=c" (count)
        : "d" (port), "0" (data), "1" (count)
        : "memory", "cc"
    );  
}
static inline void insl(uint16 port, uint32 *data, uint32 count) {
    __asm__ __volatile__ (  
        "rep; insl"  
        : "=S" (data), "=c" (count)
        : "d" (port), "0" (data), "1" (count)
        : "memory", "cc"
    );  
}