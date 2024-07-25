#include "fs/FileSystem.h"
#include "fs/VFS.h"
#include "ProcFs.h"

#include "klib/memory.h"

#include "init/Init.h"

#include "klib/stdio.h"
#include "klib/string.h"

static VFS::FileSystem* ProcFSFactory() {
    return new ProcFS();
}

static void Init() {
    VFS::FileSystemRegistry::RegisterFileSystem("procfs", ProcFSFactory);
}
REGISTER_INIT_FUNC(Init, INIT_STAGE_FSDRIVERS);

using namespace VFS;

extern struct ProcNode rootNode; // proc_dir.cpp

ProcFS::ProcFS() {
    m_RootNodeID = 0;
    this->fs_name = "procfs";
}

void ProcFS::GetSuperBlock(SuperBlock* sb) {
    sb->rootNode = m_RootNodeID;
}
void ProcFS::SetMountPoint(MountPoint* mp) {
    m_MP = mp;
}

void ProcFS::PrepareUnmount() {
}

void ProcFS::CreateNode(Node* node) {
}
void ProcFS::DestroyNode(Node* node) {
}

void ProcFS::ReadNode(uint64 id, VFS::Node* node) {
    procfs_handle_node(id, node);
}

void ProcFS::WriteNode(Node* node) {
}

void ProcFS::UpdateDir(VFS::Node* node) {
}

uint64 ProcFS::ReadNodeData(Node* node, uint64 pos, void* buffer, uint64 bufferSize) {
    return procfs_read_node(node,pos,buffer,bufferSize);
}
uint64 ProcFS::WriteNodeData(Node* node, uint64 pos, const void* buffer, uint64 bufferSize) {
    return 0;
}

void ProcFS::ClearNodeData(Node* node) {
}
