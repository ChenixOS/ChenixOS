#pragma once

#include <stdint.h>
#include <types.h>

// GPT头 "EFI PART"
#define GPT_MAGIC 0x4546492050415254
// GPT在MBR的头部
// 0x0 = 0
// 0x1 ~ 0x3 = 0x000200
// 0x4 = 0xEE
#define GPT_MBR_MAGIC 0x000200EE

struct GPT_Part_Header {
    uint64 Signature; // = GPT_MAGIC
    uint32 Revision;
    uint32 Size;
    uint32 Crc32;
    uint32 Reserved;
    uint64 LBA_Header;
    uint64 LBA_Header_2;
    uint64 GPT_First_Block; // 	The first usable block that can be contained in a GPT entry
    uint64 GPT_Last_Block; // The last usable block that can be contained in a GPT entry
    char GUID[16];
    uint64 LBA_First_Entries;
    uint32 CountOfEntryArray; // Count of Entry array (GPT_Part_entries)
    uint32 SizeOfEntry; //	Size (in bytes) of each entry in the Partition Entry array - must be a value of 128×2ⁿ where n ≥ 0 (in the past, multiples of 8 were acceptable)
    uint32 Crc32OfEntryArray;
};

struct GPT_Part_Entries {
    char TypeGUID[16];
    char GUID[16];
    uint64 LBA_START;
    uint64 LBA_END;
    uint64 ATTRIBUTE;
    char PART_NAME[72];
};
