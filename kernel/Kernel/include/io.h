#ifndef __IO_H__
#define __IO_H__

#include "types.h"

static inline void IO_Out8(uint16 port, uint8 value)
{
   __asm__ __volatile__ ("outb %0, %1" : :"a" (value), "d" (port));
}

static inline void IO_Out16(uint16 port, uint16 value)
{
   __asm__ __volatile__ ("outw %0, %1" : :"a" (value), "d" (port));
}

static inline void IO_Out32(uint16 port, uint32 value)
{
   __asm__ __volatile__ ("outl %0, %1" : :"a" (value), "d" (port));
}

static inline uint8 IO_In8(uint16 port)
{
   uint8 value;
   __asm__ __volatile__ ("inb %1, %0" :"=a" (value) :"d" (port));
   return value;
}

static inline uint16 IO_In16(uint16 port)
{
   uint16 value;
   __asm__ __volatile__ ("inw %1, %0" :"=a" (value) :"d" (port));
   return value;
}

static inline uint32 IO_In32(uint16 port)
{
   uint32 value;
   __asm__ __volatile__ ("inl %1, %0" :"=a" (value) :"d" (port));
   return value;
}

#endif /* __IO_H__ */