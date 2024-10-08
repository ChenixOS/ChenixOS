#pragma once

#include "types.h"
#include "Permissions.h"
#include "multicore/locks/StickyLock.h"
#include "multicore/locks/QueueLock.h"
#include "multicore/Atomics.h"
#include "klib/AnchorList.h"

namespace VFS {

    struct MountPoint;
    struct Directory;

    struct Node
    {
        ktl::Anchor<Node> anchor;

        uint64 id;
        MountPoint* mp;

        uint64 refCount;
        Atomic<uint64> linkCount;    // How often this node is referenced by directory entries

        bool ready;
        QueueLock readyQueue;

        enum Type {
            TYPE_FILE,              // Normal File
            TYPE_DIRECTORY,         // Directory, containing other nodes
            TYPE_DEVICE_CHAR,       // Character Device File
            TYPE_DEVICE_BLOCK,      // Block Device File
            TYPE_PIPE,              // (Named) Pipe
            TYPE_SYMLINK,           // Symbolic link
            TYPE_SOCKET,            // Socket
            TYPE_PROXY_IO           // Else of proxyfs object 
        } type;

        StickyLock dirLock;

        union {
            struct {
                Directory* cachedDir;
            } infoFolder;
            struct {
                Atomic<uint64> fileSize;
            } infoFile;
            struct {
                uint64 driverID;
                uint64 subID;
            } infoDevice;
            struct {
                char* linkPath;
            } infoSymlink;
        };

        uint64 ownerUID;
        uint64 ownerGID;
        Permissions permissions;

        void* fsData;
    };

}