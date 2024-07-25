#pragma once

#include <fs/FileSystem.h>
#include "fs/MountPoint.h"

namespace ProxyFs {

class ProxyFS : public VFS::FileSystem {
public:
    void GetSuperBlock(VFS::SuperBlock* sb) override;
    void SetMountPoint(VFS::MountPoint* mp) override;
    void PrepareUnmount() override;

    void CreateNode(VFS::Node* node) override;
    void DestroyNode(VFS::Node* node) override;

    void UpdateDir(VFS::Node* node) override;
    
    void ReadNode(uint64 id, VFS::Node* node) override;
    void WriteNode(VFS::Node* node) override;

    virtual uint64 ReadNodeData(VFS::Node* node, uint64 pos, void* buffer, uint64 bufferSize) override;
    virtual uint64 WriteNodeData(VFS::Node* node, uint64 pos, const void* buffer, uint64 bufferSize) override;
    virtual void ClearNodeData(VFS::Node* node) override;
};

class IoObject {
public:
    IoObject(int type);

    virtual uint64 Read(uint64 pos, void* buffer, uint64 bufferSize) = 0;
    virtual uint64 Write(uint64 pos, const void* buffer, uint64 bufferSize) = 0;
    virtual void Clear() = 0;
    virtual void Close() = 0;

    int GetType();
private:
    int objType = VFS::Node::TYPE_PROXY_IO;
};

};