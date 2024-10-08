#include "FileSystem.h"

#include "multicore/locks/StickyLock.h"
#include "klib/AnchorList.h"
#include "klib/string.h"

namespace VFS {

    struct FSEntry {
        ktl::Anchor<FSEntry> anchor;

        union {
            FileSystemFactory factory;
            FileSystemFactoryDev factoryDev;
        };
        bool needsDev;
        const char* id;
    };

    static StickyLock g_Lock;
    static ktl::AnchorList<FSEntry, &FSEntry::anchor> g_FileSystems;

    void FileSystemRegistry::RegisterFileSystem(const char* id, FileSystemFactory factory) {
        FSEntry* entry = new FSEntry();
        entry->factory = factory;
        entry->needsDev = false;
        entry->id = id;

        g_Lock.Spinlock();
        g_FileSystems.push_back(entry);
        g_Lock.Unlock();
    }
    void FileSystemRegistry::RegisterFileSystem(const char* id, FileSystemFactoryDev factory) {
        FSEntry* entry = new FSEntry();
        entry->factoryDev = factory;
        entry->needsDev = true;
        entry->id = id;

        g_Lock.Spinlock();
        g_FileSystems.push_back(entry);
        g_Lock.Unlock();
    }

    void FileSystemRegistry::UnregisterFileSystem(const char* id) {
        g_Lock.Spinlock();
        for(auto a = g_FileSystems.begin(); a != g_FileSystems.end(); ++a) {
            if(kstrcmp(a->id, id) == 0) {
                g_FileSystems.erase(a);
                delete &*a;
                g_Lock.Unlock();
                return;
            }
        }
        g_Lock.Unlock();
    }

    FileSystem* FileSystemRegistry::CreateFileSystem(const char* id) {
        g_Lock.Spinlock();
        for(const FSEntry& entry : g_FileSystems) {
            if(kstrcmp(entry.id, id) == 0) {
                if(entry.needsDev) {
                    g_Lock.Unlock();
                    return nullptr;
                }

                FileSystemFactory factory = entry.factory;
                g_Lock.Unlock();
                FileSystem* res = factory();
                return res;
            }
        }

        g_Lock.Unlock();
        return nullptr;
    }
    FileSystem* FileSystemRegistry::CreateFileSystem(const char* id, BlockDeviceDriver* devDriver, uint64 devID) {
        g_Lock.Spinlock();
        for(const FSEntry& entry : g_FileSystems) {
            if(kstrcmp(entry.id, id) == 0) {
                if(!entry.needsDev) {
                    g_Lock.Unlock();
                    return nullptr;
                }

                FileSystemFactoryDev factory = entry.factoryDev;
                g_Lock.Unlock();
                FileSystem* res = factory(devDriver, devID);
                return res;
            }
        }

        g_Lock.Unlock();
        return nullptr;
    }

}