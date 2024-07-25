#include "fs/VFS.h"
#include "fs/procfs/ProcFs.h"
#include "init/Init.h"

static void print_mount_info(VFS::MountPoint* point,StringBuffer *out) {
    VFS::FileSystem* fs = point->fs;

    if(fs == nullptr)
        return;

    point->childMountLock.Spinlock();

    out->append((fs->GetFileSystemName() == nullptr) ? "none" : fs->GetFileSystemName());
    out->append(" /");
    out->append(point->path);
    out->append("\n");

    for(VFS::MountPoint& mp : point->childMounts) {
        print_mount_info(&mp, out);
    }
    point->childMountLock.Unlock();
}

void procfs_mount_info(ProcNode *node, VFS::Node *in, StringBuffer *buffer) {
    VFS::MountPoint* root = VFS::GetRootMountPoint();
    print_mount_info(root,buffer);
    // buffer->append("Hello World!");
}

static void MyInit() {
    procfs_new_string_callback_node("mounts",procfs_mount_info);
}

REGISTER_INIT_FUNC(MyInit, INIT_STAGE_DELAY);