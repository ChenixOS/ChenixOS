#include "global/KernelHeader.h"

#include "HeaderInfo.h"
#include "string.h"

#include "fs/procfs/ProcFs.h"
#include "init/Init.h"

namespace HeaderInfo {
    static StringBuffer* g_InfoBuffer;

    void Init(KernelHeader* my_KernelHeader) {
        StringBuffer* buffer = new StringBuffer();

        buffer->append("Kernel Info: \n\n");
        buffer->format("Kernel Header: %X\n", my_KernelHeader);
        buffer->format("Kernel Base Address: %X\n", my_KernelHeader->kernelImage.buffer);
        buffer->format("Kernel Used Page: %d\n", my_KernelHeader->kernelImage.numPages);
        buffer->format("Ramdisk Base Address: %X\n", my_KernelHeader->ramdiskImage.buffer);
        buffer->format("Kernel Used Page: %d\n", my_KernelHeader->ramdiskImage.numPages);
        buffer->format("High Memory Base: %X\n", my_KernelHeader->highMemoryBase);
        buffer->format("Page Buffer: %X-%X (%d KB)\n", my_KernelHeader->pageBuffer, my_KernelHeader->pageBuffer + (my_KernelHeader->pageBufferPages * 4096), my_KernelHeader->pageBufferPages * 4);

        buffer->append("\nMemory Mapper: \n\n");
        for(PhysicalMapSegment* seg = my_KernelHeader->physMapStart; seg != nullptr;) {
            buffer->format("%X -> 0x%X-0x%X : total %d KB \n", seg, seg->base, seg->base + (seg->numPages * 4096), seg->numPages * 4);
            if(seg == my_KernelHeader->physMapEnd) {
                break;
            }
            seg = seg->next;
        }

        buffer->append("\nVGA Screen Info: \n\n");
        buffer->format("Screen Type: %s\n", (my_KernelHeader->screenColorsInverted) ? "BGR" : "RGB");
        buffer->format("Screen Size: %dx%d\n",my_KernelHeader->screenWidth,my_KernelHeader->screenHeight);
        buffer->format("Screen Buffer: %X-%X (%d KB)\n", my_KernelHeader->screenBuffer, my_KernelHeader->screenBuffer + (my_KernelHeader->screenBufferPages * 4096), my_KernelHeader->screenBufferPages * 4);

        g_InfoBuffer = buffer;
    }

    static void procfs_show_header(ProcNode *node, VFS::Node *in, StringBuffer *buffer) 
    {
        buffer->copy(g_InfoBuffer);
    }

    static void MyInit() {
        procfs_new_string_callback_node("bootinfo",procfs_show_header);
    }
    REGISTER_INIT_FUNC(MyInit, INIT_STAGE_DELAY);

    void AppendToHeaderInfo(const char* str) {
        g_InfoBuffer->append(str);
    }
    EXPORT_DEF_SYMBOL(AppendToHeaderInfo);
}
