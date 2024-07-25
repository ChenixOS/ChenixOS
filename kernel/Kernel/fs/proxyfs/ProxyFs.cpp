#include "ProxyFs.h"
#include "init/Init.h"
#include "fs/VFS.h"

#include "task/Scheduler.h"

using namespace VFS;

namespace ProxyFs {
    struct ProxyNode {
        IoObject* obj;
    };

    static ProxyFS* proxyFs = nullptr;
    static MountPoint* proxyFsMp = nullptr;

    // 初始化
    void initProxyFs() {
        proxyFs = new ProxyFS();
        proxyFsMp = new MountPoint();
        proxyFsMp->fs = proxyFs;
    }

    REGISTER_INIT_FUNC(initProxyFs,INIT_STAGE_FSDRIVERS);
    
    // ======================================================
    
    void ProxyFS::GetSuperBlock(SuperBlock* sb) { }
    void ProxyFS::SetMountPoint(MountPoint* mp) { }
    void ProxyFS::PrepareUnmount() { }

    void ProxyFS::UpdateDir(Node* node) { }

    void ProxyFS::ReadNode(uint64 id, Node* node) { }
    void ProxyFS::WriteNode(Node* node) { }

    void ProxyFS::CreateNode(Node* node) {
        node->linkCount = 0;
    }

    // ======================================================

    void ProxyFS::DestroyNode(Node* node) {
        IoObject* obj = (IoObject*)node->id;
        obj->Close();
    }

    uint64 ProxyFS::ReadNodeData(VFS::Node* node, uint64 pos, void* buffer, uint64 bufferSize) {
        IoObject* obj = (IoObject*)node->id;
        return obj->Read(pos,buffer,bufferSize);
    }

    uint64 ProxyFS::WriteNodeData(VFS::Node* node, uint64 pos,const void* buffer, uint64 bufferSize) {
        IoObject* obj = (IoObject*)node->id;
        return obj->Write(pos,buffer,bufferSize);
    }

    void ProxyFS::ClearNodeData(Node* node) {
        IoObject* obj = (IoObject*)node->id;
        obj->Clear();
    }

    // ==============================================================

    uint64 CreateProxyFileDesc(IoObject* obj) {
        auto newNode = new Node();
        MountPoint* mp = proxyFsMp;

        mp->fs->CreateNode(newNode);
        newNode->refCount = 1;
        newNode->mp = mp;
        newNode->dirLock.Unlock_Raw();

        mp->nodeCacheLock.Spinlock();
        mp->nodeCache.push_back(newNode);
        mp->nodeCacheLock.Unlock();

        newNode->id = (uint64)obj;
        newNode->type = (VFS::Node::Type)obj->GetType();

        FileDescriptor* desc = new FileDescriptor();
        desc->node = newNode;
        desc->pos = 0;
        desc->refCount = 1;
        desc->permissions = Permissions::Write;

        return Scheduler::ThreadAddFileDescriptor((uint64)desc);
    }


    // ===============================================================

    IoObject::IoObject(int type) {
        this->objType = type;
    }

    int IoObject::GetType() {
        return this->objType;
    }


};