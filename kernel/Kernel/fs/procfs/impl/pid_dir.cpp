#include "fs/procfs/ProcFs.h"
#include "string.h"
#include "stl/vector.h"
#include "stl/map.h"
#include "task/Scheduler.h"
#include "init/Init.h"

using namespace VFS;
using namespace Scheduler;

ProcNode headNode;
Vector<ProcNode*> pid_dir_nodes;
StringTable<ProcNode*> pid_dir_tables;

void pid_dir_stat_func(IDInfo* info,VFS::Node* out) {
    if(info->nodeId == 0) {
        out->ready = true;
        out->type = Node::Type::TYPE_DIRECTORY;
        out->ownerGID = 0;
        out->ownerUID = 0;
        out->permissions.ownerPermissions = Permissions::Read | Permissions::Execute;
        out->permissions.groupPermissions = Permissions::Read | Permissions::Execute;
        out->permissions.otherPermissions = Permissions::Read | Permissions::Execute;
        out->permissions.specialFlags = 0;

        Directory* dir = Directory::Create(pid_dir_tables.count());

        for(auto &kv : pid_dir_tables.pairs()) {
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
            
            vfsEntry->nodeID = info_to_id(info->pid,itemId);
            kstrcpy(vfsEntry->name, name);
        }

        out->infoFolder.cachedDir = dir;
    } else {
        if(info->nodeId < pid_dir_nodes.count()) {
            ProcNode* node = pid_dir_nodes[info->nodeId];
            if(node != nullptr && node->stat_func != nullptr) {
                node->stat_func(node, out);
            }
        } else {
            out->ready = false;
        }
    }
}

int64 pid_dir_file_read_func(IDInfo* info, VFS::Node* in, uint64 pos, void* buffer, uint64 bufferSize) {
    if(info->nodeId < pid_dir_nodes.count()) {
        ProcNode* node = pid_dir_nodes[info->nodeId];
        if(node != nullptr && node->stat_func != nullptr) {
            return node->read_func(node, in, pos, buffer, bufferSize);
        }
    }
    return 0;
}


/**
 * @brief 添加ProcFs的pid目录的节点
 * 
 * @param name 名称
 * @param cookie 传入参数
 * @param stat_func 信息回调
 * @param read_func 读取回调
 */
void procfs_new_rootdir_pid_node(const char* name,
    void* cookie,
    proc_file_stat_func_t stat_func,
    proc_file_read_func_t read_func) {
        ProcNode* node = new ProcNode;
        node->cookie = cookie;
        node->stat_func = stat_func;
        node->read_func = read_func;

        node->subId = pid_dir_nodes.count();

        pid_dir_nodes.push_back(node);
        pid_dir_tables.put(name,node);
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
void procfs_new_string_callback_pid_node(const char* name,proc_string_callback_func_t callback) {
    procfs_new_rootdir_pid_node(name,(void*)callback,&none_stat_func,&string_callback_read_func);
}

// ===============================================================================
// /proc/<pid>/status

void procfs_pid_status_info(ProcNode *node, VFS::Node *in, StringBuffer *buffer) {
    struct IDInfo info;

    parse_id_info(in->id, &info);

    struct ThreadInfo threadInfo;
    bool ready = Scheduler::FindProcessMainThreadInfoAndCopy(info.pid,&threadInfo);

    char strbuf[32];

    if(ready) {
        buffer->append("Name: ");
        buffer->append(threadInfo.name);
        buffer->append("\n");

        buffer->append("Status: ");
        switch(threadInfo.state.type) {
            case ThreadState::READY:
                buffer->append("R (running)");
                break;
            case ThreadState::EXITED:
            case ThreadState::FINISHED:
                buffer->append("E (exit)");
                break;
            case ThreadState::JOIN:
            case ThreadState::QUEUE_LOCK:
            case ThreadState::SLEEP:
                buffer->append("S (sleeping)");
                break;
            default:
                buffer->append("Unknown");
                break;
        }
        buffer->append("\n");

        buffer->append("Pid: ");
        buffer->append(itoa(threadInfo.pid,strbuf,10));
        buffer->append("\n");

        buffer->append("Ppid: ");
        buffer->append(itoa(threadInfo.ppid,strbuf,10));
        buffer->append("\n");

        buffer->append("Uid: ");
        buffer->append(itoa(threadInfo.uid,strbuf,10));
        buffer->append("\n");

        buffer->append("Gid: ");
        buffer->append(itoa(threadInfo.gid,strbuf,10));
        buffer->append("\n");

        // 以下是需要锁的操作
        Scheduler::GlobalLock();

        int i = 1; // 算上主进程
        for(auto &none : threadInfo.childThreads) {
            i++;
        }
        buffer->append("Threads: ");
        buffer->append(itoa(i,strbuf,10));
        buffer->append("\n");

        Scheduler::GlobalUnlock();
    }
}

void procfs_pid_exe_info(ProcNode *node, VFS::Node *in, StringBuffer *buffer) {
    struct IDInfo info;

    parse_id_info(in->id, &info);

    struct ThreadInfo threadInfo;
    bool ready = Scheduler::FindProcessMainThreadInfoAndCopy(info.pid,&threadInfo);

    if(ready) {
        buffer->append("/");
        buffer->append(threadInfo.exe_path);
    }
}

// =========================================================

// 实现/proc/self的指向

void proc_self_stat_func(ProcNode* node,VFS::Node* out) {
    out->ready = true;
    out->type = Node::Type::TYPE_SYMLINK;
    out->ownerGID = 0;
    out->ownerUID = 0;
    out->permissions.ownerPermissions = Permissions::Read | Permissions::Execute;
    out->permissions.groupPermissions = Permissions::Read | Permissions::Execute;
    out->permissions.otherPermissions = Permissions::Read | Permissions::Execute;
    out->permissions.specialFlags = 0;

    StringBuffer* sb = new StringBuffer();
    sb->append("/proc/");
    sb->append(Scheduler::GetCurrentThreadInfo()->pid);

    out->infoSymlink.linkPath = sb->to_string();

    delete sb;
}


// 初始化代码

static void MyInit() {
    pid_dir_nodes.push_back(&headNode);

    procfs_new_rootdir_sub_node("self",nullptr,&proc_self_stat_func,nullptr);

    procfs_new_string_callback_pid_node("status",&procfs_pid_status_info);
    procfs_new_string_callback_pid_node("exe",&procfs_pid_exe_info);
}

REGISTER_INIT_FUNC(MyInit, INIT_STAGE_FSDRIVERS);