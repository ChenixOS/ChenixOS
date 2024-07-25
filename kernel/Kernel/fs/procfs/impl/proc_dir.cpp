#include "fs/procfs/ProcFs.h"
#include "string.h"
#include "stl/vector.h"
#include "stl/map.h"
#include "task/Scheduler.h"
#include "init/Init.h"
#include "fs/VFS.h"
#include "fs/MountPoint.h"

#include "stdio.h"

using namespace VFS;
using namespace Scheduler;

ProcNode rootNode; // 根Node占位用
Vector<ProcNode*> proc_dir_nodes; // 所有节点与ID的映射关系
StringTable<ProcNode*> proc_dir_tables; // proc根目录的节点(非pid节点)映射关系

// 处理文件系统获取信息
void procfs_handle_node(uint64 id,VFS::Node* out) {
    if(id == 0) {
        out->ready = true;
        out->type = Node::Type::TYPE_DIRECTORY;
        out->ownerGID = 0;
        out->ownerUID = 0;
        out->permissions.ownerPermissions = Permissions::Read | Permissions::Execute;
        out->permissions.groupPermissions = Permissions::Read | Permissions::Execute;
        out->permissions.otherPermissions = Permissions::Read | Permissions::Execute;
        out->permissions.specialFlags = 0;

        // 生成PID节点
        Vector<int64>* pidList = ListAllProcessId();
        Directory* dir = Directory::Create(pidList->count());

        for(size_t i = 0;i < pidList->count();i++) {
            int64 pid = (*pidList)[i];
            
            // 生成Entry
            DirectoryEntry* vfsEntry;
            Directory::AddEntry(&dir, &vfsEntry);

            vfsEntry->nodeID = info_to_id(pid, 0);
            itoa(pid, vfsEntry->name, 10);
        }
        
        delete pidList;

        // 生成普通节点
        for(auto &kv : proc_dir_tables.pairs()) {
            const char* name = kv.name;
            ProcNode* value = kv.value;
            if(name == nullptr || value == nullptr) {
                continue;
            }

            // size_t itemId = proc_dir_nodes.find(value);
            size_t itemId = value->subId;
            if(itemId == -1) {
                continue;
            }

            // 生成Entry
            DirectoryEntry* vfsEntry;
            Directory::AddEntry(&dir, &vfsEntry);
            
            vfsEntry->nodeID = info_to_id(0,itemId);
            kstrcpy(vfsEntry->name, name);
        }

        
        // for(size_t i = 0;i < proc_dir_tables.count();i++) {    
        // }

        out->infoFolder.cachedDir = dir;
        out->fsData = &rootNode;

    } else {
        struct IDInfo info;

        parse_id_info(id,&info);

        if(info.pid == 0) {
            if(info.nodeId < proc_dir_nodes.count()) {
                ProcNode* node = proc_dir_nodes[info.nodeId];
                if(node != nullptr) {
                    node->stat_func(node,out);
                    out->fsData = node;
                }
            }
        } else {
            pid_dir_stat_func(&info,out);
        }
    }
}

uint64 procfs_read_node(Node* node, uint64 pos, void* buffer, uint64 bufferSize) {
    if(node->fsData != nullptr) {
        ProcNode* procNode = (ProcNode*) node->fsData;
        if(procNode->read_func == nullptr)
            return 0;
        return procNode->read_func(procNode, node, pos, buffer, bufferSize);
    } else {
        struct IDInfo info;

        parse_id_info(node->id,&info);

        return pid_dir_file_read_func(&info, node, pos, buffer, bufferSize);
    }
    return 0;
}

/**
 * @brief 添加ProcFs根目录的节点
 * 
 * @param name 名称
 * @param cookie 传入参数
 * @param stat_func 信息回调
 * @param read_func 读取回调
 */
void procfs_new_rootdir_sub_node(const char* name,
    void* cookie,
    proc_file_stat_func_t stat_func,
    proc_file_read_func_t read_func) {
        ProcNode* node = new ProcNode;
        node->cookie = cookie;
        node->stat_func = stat_func;
        node->read_func = read_func;

        node->subId = proc_dir_nodes.count();

        proc_dir_nodes.push_back(node);
        proc_dir_tables.put(name,node);
}


// ===========================================

static void none_stat_func(ProcNode* node,VFS::Node* out) {
    out->ready = true;
    out->type = Node::Type::TYPE_FILE;
    out->ownerGID = 0;
    out->ownerUID = 0;
    out->permissions.ownerPermissions = Permissions::Read | Permissions::Execute;
    out->permissions.groupPermissions = Permissions::Read | Permissions::Execute;
    out->permissions.otherPermissions = Permissions::Read | Permissions::Execute;
    out->permissions.specialFlags = 0;
}

static uint64 string_read_func(ProcNode* node,VFS::Node* in, uint64 pos, void* buffer, uint64 bufferSize) {
    size_t len = kstrlen((char*)node->cookie);
    if(len == 0)
        return 0;
    if(pos >= len)
        return 0;
    size_t copySize = ((len - pos) >= bufferSize) ? bufferSize : (len - pos);

    kmemcpy_usersafe(buffer,(node->cookie + pos),copySize);
    return copySize;
}

// 这是创建固定内容的proc目录文件
void procfs_new_string_node(const char* name,char* text) {
    procfs_new_rootdir_sub_node(name,text,&none_stat_func,&string_read_func);
}

// ====================================================

static uint64 string_callback_read_func(ProcNode* node,VFS::Node* in, uint64 pos, void* buffer, uint64 bufferSize) {
    proc_string_callback_func_t func = (proc_string_callback_func_t) node->cookie;
    if(func == nullptr) {
        return 0;
    }

    StringBuffer* sb = new StringBuffer();
    func(node,in,sb);

    size_t copySize = 0;
    // 开始处理数据
    if(sb->count() > 0) {
        const char* text = sb->c_str();
        size_t len = kstrlen(text);
        
        if(len > 0 && pos < len) {
            copySize = ((len - pos) >= bufferSize) ? bufferSize : (len - pos);
            kmemcpy_usersafe(buffer,(text + pos),copySize);
        }
    }
    
    delete sb;

    return copySize;
}

// 这是创建固定内容的proc目录文件
void procfs_new_string_callback_node(const char* name,proc_string_callback_func_t callback) {
    procfs_new_rootdir_sub_node(name,(void*)callback,&none_stat_func,&string_callback_read_func);
}

// ===========================================
// 这些是测试代码

static void MyInit() {
    proc_dir_nodes.push_back(&rootNode);
    // procfs_new_rootdir_sub_node("hello",nullptr,&none_stat_func,nullptr);
    procfs_new_string_node("hello","Hello World");
}

REGISTER_INIT_FUNC(MyInit, INIT_STAGE_FSDRIVERS);