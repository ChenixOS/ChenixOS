#pragma once

#include "fs/FileSystem.h"
#include "stl/stringbuffer.h"
#include "fs/MountPoint.h"

struct ProcNode;

typedef void (*proc_file_stat_func_t)(ProcNode* node,VFS::Node* out);
typedef uint64 (*proc_file_read_func_t)(ProcNode* node,VFS::Node* in, uint64 pos, void* buffer, uint64 bufferSize);
typedef void (*proc_string_callback_func_t)(ProcNode* node,VFS::Node* in,StringBuffer* buffer);

struct ProcNode {
    VFS::Node::Type type;
    void* cookie;
    uint16 subId;
    proc_file_stat_func_t stat_func;
    proc_file_read_func_t read_func;
};

class ProcFS : public VFS::FileSystem {
public:
    ProcFS();

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

private:
    uint64 m_RootNodeID;
    VFS::MountPoint* m_MP;
};

// proc_dir.cpp

// ID信息
struct IDInfo {
    uint32 pid;
    uint16 nodeId;
    uint16 subId;
};

// inline
inline void parse_id_info(uint64 id,struct IDInfo* info) {
    kmemcpy((void*)info,(void*)&id,sizeof(uint64));
}

inline uint64 info_to_id(uint32 pid,uint16 nodeId,uint16 subId) {
    uint64 result = 0;
    struct IDInfo* info = (struct IDInfo*)&result;

    info -> pid = pid;
    info -> nodeId = nodeId;
    info -> subId = subId;

    return result;
}

inline uint64 info_to_id(uint32 pid,uint16 nodeId) {
    return info_to_id(pid,nodeId,0);
}

void procfs_handle_node(uint64 id,VFS::Node* out);
uint64 procfs_read_node(VFS::Node* node, uint64 pos, void* buffer, uint64 bufferSize);
void procfs_new_rootdir_sub_node(const char* name,
    void* cookie,
    proc_file_stat_func_t stat_func,
    proc_file_read_func_t read_func);
void procfs_new_string_node(const char* name,char* text);
void procfs_new_string_callback_node(const char* name,proc_string_callback_func_t callback);


// pid_dir.cpp
void procfs_new_rootdir_pid_node(const char* name,
    void* cookie,
    proc_file_stat_func_t stat_func,
    proc_file_read_func_t read_func);
void pid_dir_stat_func(IDInfo* info,VFS::Node* out);
int64 pid_dir_file_read_func(IDInfo* info, VFS::Node* in, uint64 pos, void* buffer, uint64 bufferSize);

void procfs_pid_exe_info(ProcNode *node, VFS::Node *in, StringBuffer *buffer);
