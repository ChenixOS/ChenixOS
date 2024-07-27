#include "DevFS.h"

#include "multicore/locks/StickyLock.h"
#include "klib/string.h"
#include "init/Init.h"
#include "klib/stdio.h"

#include <vector>

#include "kernel/SymbolTable.h"

struct DevReg {
    const char* name;
    bool blockDev;
    uint64 driverID;
    uint64 devID;

    uint64 index;
};

static StickyLock g_Lock;

static std::vector<DevReg> g_Devices;
static VFS::MountPoint* g_MP = nullptr;

static VFS::Directory* g_CurrentDir = nullptr;
static VFS::Directory* g_NewDir = nullptr;

static void CheckAndCopyEntry() {
    if(g_NewDir == nullptr) {
        g_NewDir = VFS::Directory::Create(g_CurrentDir->numEntries + 5);
        VFS::Directory::CopyTo(&g_CurrentDir,&g_NewDir);
    }
}

void DevFS::RegisterCharDevice(const char* name, uint64 driverID, uint64 devID) {
    g_Lock.Spinlock();

    CheckAndCopyEntry();

    VFS::DirectoryEntry* entry;
    VFS::Directory::AddEntry(&g_NewDir, &entry);

    kstrcpy(entry->name, name);
    entry->nodeID = g_Devices.size() + 1;

    g_Devices.push_back({ name, false, driverID, devID, g_Devices.size() });

    g_Lock.Unlock();
}

EXPORT_CPP_SYMBOL("DevFS::RegisterCharDevice", RegisterCharDevice, DevFS::RegisterCharDevice);

void DevFS::RegisterBlockDevice(const char* name, uint64 driverID, uint64 devID) {
    g_Lock.Spinlock();

    CheckAndCopyEntry();

    VFS::DirectoryEntry* entry;
    VFS::Directory::AddEntry(&g_NewDir, &entry);

    kstrcpy(entry->name, name);
    entry->nodeID = g_Devices.size() + 1;

    g_Devices.push_back({ name, true, driverID, devID, g_Devices.size() });

    g_Lock.Unlock();
}

EXPORT_CPP_SYMBOL("DevFS::RegisterBlockDevice", RegisterBlockDevice, DevFS::RegisterBlockDevice);

void DevFS::UnregisterDevice(uint64 driverID, uint64 devID) {
    
}

EXPORT_CPP_SYMBOL("DevFS::UnregisterDevice", UnregisterDevice, DevFS::UnregisterDevice);

DevFS::DevFS() {
    g_CurrentDir = VFS::Directory::Create(30);
    this->fs_name = "devfs";
}

void DevFS::GetSuperBlock(VFS::SuperBlock* sb) {
    sb->rootNode = 0xFFFF;
}
void DevFS::SetMountPoint(VFS::MountPoint* mp) { g_MP = mp; }
void DevFS::PrepareUnmount() { }

void DevFS::CreateNode(VFS::Node* node) { }
void DevFS::DestroyNode(VFS::Node* node) { }

void DevFS::UpdateDir(VFS::Node* node) {
    g_Lock.Spinlock();

    if(g_NewDir != nullptr) {
        VFS::Directory::Destroy(g_CurrentDir);
        g_CurrentDir = g_NewDir;
        g_NewDir =  nullptr;
    }

    node->infoFolder.cachedDir = g_CurrentDir;

    g_Lock.Unlock();
}

void DevFS::ReadNode(uint64 id, VFS::Node* node) {
    if(id == 0xFFFF) {
        node->type = VFS::Node::TYPE_DIRECTORY;
        node->ownerUID = 0;
        node->ownerGID = 0;
        node->permissions = { 5, 5, 5 };
        node->id = 0xFFFF;
        node->linkCount = 1;

        g_Lock.Spinlock();
        
        if(g_NewDir != nullptr) {
            VFS::Directory::Destroy(g_CurrentDir);
            g_CurrentDir = g_NewDir;
            g_NewDir = nullptr;
        }

        node->infoFolder.cachedDir = g_CurrentDir;

        g_Lock.Unlock();
    } else {
        g_Lock.Spinlock();
        const DevReg& dr = g_Devices[id - 1];

        node->type = dr.blockDev ? VFS::Node::TYPE_DEVICE_BLOCK : VFS::Node::TYPE_DEVICE_CHAR;
        node->infoDevice.driverID = dr.driverID;
        node->infoDevice.subID = dr.devID;
        node->ownerGID = 0;
        node->ownerUID = 0;
        node->permissions = { 1, 1, 1 };
        node->id = id;
        node->linkCount = 1;

        g_Lock.Unlock();
    }
}

void DevFS::WriteNode(VFS::Node* node) { }

uint64 DevFS::ReadNodeData(VFS::Node* node, uint64 pos, void* buffer, uint64 bufferSize) { return 0; }
uint64 DevFS::WriteNodeData(VFS::Node* node, uint64 pos, const void* buffer, uint64 bufferSize) { return 0; }
void DevFS::ClearNodeData(VFS::Node* node) { }


static VFS::FileSystem* DevFSFactory() {
    return new DevFS();
}

static void Init() {
    VFS::FileSystemRegistry::RegisterFileSystem("devFS", DevFSFactory);
}
REGISTER_INIT_FUNC(Init, INIT_STAGE_FSDRIVERS);

// =======================================================================

#include "fs/procfs/ProcFs.h"
#include "init/Init.h"

void devfs_show_devices(ProcNode *node, VFS::Node *in, StringBuffer *buffer) 
{
    char buf[255];

    g_Lock.Spinlock();

    buffer->append("Character devices:\n");
    for(auto &driver : g_Devices) {
        if(!driver.blockDev) {
            buffer->append(itoa(driver.driverID, buf, 10));
            buffer->append(" ");
            buffer->append(driver.name);
            buffer->append("\n");
        }
    }

    buffer->append("\nBlock devices:\n");
    for(auto &driver : g_Devices) {
        if(driver.blockDev) {
            buffer->append(itoa(driver.driverID, buf, 10));
            buffer->append(" ");
            buffer->append(driver.name);
            buffer->append("\n");
        }
    }

    g_Lock.Unlock();
}

static void MyInit() {
    procfs_new_string_callback_node("devices",devfs_show_devices);
}

REGISTER_INIT_FUNC(MyInit, INIT_STAGE_DELAY);