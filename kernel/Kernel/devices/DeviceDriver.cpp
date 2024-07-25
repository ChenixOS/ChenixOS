#include "DeviceDriver.h"

#include "klib/AnchorList.h"
#include "task/Scheduler.h"
#include "multicore/locks/QueueLock.h"
#include "fs/VFS.h"
#include "klib/string.h"
#include "klib/memory.h"

#include <new>

DeviceDriver::DeviceDriver(Type type, const char* name) 
    : m_Type(type), m_Name(name)
{
    m_ID = DeviceDriverRegistry::RegisterDriver(this);
}

CharDeviceDriver::CharDeviceDriver(const char* name) 
    : DeviceDriver(TYPE_CHAR, name)
{ }

BlockDeviceDriver::BlockDeviceDriver(const char* name)
    : DeviceDriver(TYPE_BLOCK, name)
{ }


struct CachedBlock {
    uint64 subID;
    uint64 blockID;
    uint64 refCount;
    QueueLock dataLock;
    char data[];
};

static bool GetCachedBlock(uint64 subID, uint64 blockID, uint64 blockSize, std::vector<CachedBlock*>& cache, CachedBlock** block) {
    for(CachedBlock* cb : cache) {
        if(cb->subID == subID && cb->blockID == blockID) {
            cb->refCount++;
            *block = cb;
            return true;
        }
    }

    char* buffer = new char[sizeof(CachedBlock) + blockSize];
    CachedBlock* cb = new(buffer) CachedBlock();
    cb->subID = subID;
    cb->blockID = blockID;
    cb->refCount = 1;
    cache.push_back(cb);
    *block = cb;
    return false;
}
static void ReleaseCachedBlock(CachedBlock* cb, std::vector<CachedBlock*>& cache) {
    cb->refCount--;
    if(cb->refCount == 0) {
        // TODO: Cache clean logic
    }
}

/*
uint64 BlockDeviceDriver::GetData(uint64 subID, uint64 pos, void* buffer, uint64 bufferSize) {
    char* realBuffer = (char*)buffer;

    uint64 startBlock = pos / GetBlockSize(subID);
    uint64 endBlock = (pos + bufferSize) / GetBlockSize(subID);
    uint64 numBlocks = endBlock - startBlock + 1;

    for(uint64 b = 0; b < numBlocks; b++) {
        uint64 blockID = startBlock + b;
        
        m_CacheLock.Spinlock();
        CachedBlock* cb;
        if(!GetCachedBlock(subID, blockID, GetBlockSize(subID), m_Cache, &cb)) {
            m_CacheLock.Unlock();
            cb->dataLock.Lock();

            Atomic<uint64> finished;
            Atomic<uint64> success;
            finished = 0;
            success = 0;
            ScheduleOperation(subID, blockID, 1, false, cb->data, &finished, &success);
            while(finished.Read() == 0) ;
                // TODO: Yield
            
        } else {
            m_CacheLock.Unlock();
            cb->dataLock.Lock();
        }

        uint64 offs = pos - (blockID * GetBlockSize(subID));
        uint64 rem = GetBlockSize(subID) - offs;
        if(rem > bufferSize)
            rem = bufferSize;

        if(!kmemcpy_usersafe(realBuffer, cb->data + offs, rem)) {
            cb->dataLock.Unlock();
            m_CacheLock.Spinlock();
            ReleaseCachedBlock(cb, m_Cache);
            m_CacheLock.Unlock();
            return ErrorInvalidBuffer;
        }
        cb->dataLock.Unlock();
        m_CacheLock.Spinlock();
        ReleaseCachedBlock(cb, m_Cache);
        m_CacheLock.Unlock();

        pos += rem;
        realBuffer += rem;
        bufferSize -= rem;
    }

    return 0;
}*/

uint64 BlockDeviceDriver::GetData(uint64 subID, uint64 pos, void* buffer, uint64 bufferSize) {
    char* realBuffer = (char*)buffer;

    uint64 startBlock = pos / GetBlockSize(subID);
    uint64 endBlock = (pos + bufferSize - 1) / GetBlockSize(subID); // 确保包含最后一个可能的部分块
    uint64 numBlocks = endBlock - startBlock + 1;

    for (uint64 b = 0; b < numBlocks; b++) {
        uint64 blockID = startBlock + b;
        uint64 blockOffset = pos - (blockID * GetBlockSize(subID));
        uint64 blockSize = GetBlockSize(subID);
        uint64 copySize = std::min(blockSize - blockOffset, bufferSize);

        CachedBlock* cb;
        m_CacheLock.Spinlock();
        if (!GetCachedBlock(subID, blockID, blockSize, m_Cache, &cb)) {
            m_CacheLock.Unlock();

            // 缓存未命中，调度操作
            Atomic<uint64> finished{0}; // 任务是否执行完毕，有些任务可能是异步的
            Atomic<uint64> success{0}; // 任务是否成功，异步执行失败请设置为0，成功设置为1
            ScheduleOperation(subID, blockID, 1, false, cb->data, &finished, &success);

            // 等待操作完成
            while (finished.Read() == 0) {
                // TODO: Yield 或使用更高效的等待机制
                Scheduler::Yield();
            }

            if (success.Read() == 0) {
                // 操作失败，返回错误
                return -1; // 假设这是一个适当的错误代码
            }

            m_CacheLock.Spinlock(); // 重新锁定以处理缓存块
        }

        // 从缓存块复制数据到目标缓冲区
        if (!kmemcpy_usersafe(realBuffer, cb->data + blockOffset, copySize)) {
            // 复制失败，返回错误
            m_CacheLock.Unlock();
            ReleaseCachedBlock(cb, m_Cache); // 释放缓存块
            return ErrorInvalidBuffer;
        }

        m_CacheLock.Unlock(); // 释放缓存锁
        ReleaseCachedBlock(cb, m_Cache); // 在操作成功后释放缓存块

        pos += copySize;
        realBuffer += copySize;
        bufferSize -= copySize;

        if (bufferSize == 0) {
            // 已经复制了所有请求的数据
            break;
        }
    }

    return 0; // 成功
}

uint64 BlockDeviceDriver::SetData(uint64 subID, uint64 pos, const void* buffer, uint64 bufferSize) {
    char* realBuffer = (char*)buffer;

    uint64 startBlock = pos / GetBlockSize(subID);
    uint64 endBlock = (pos + bufferSize) / GetBlockSize(subID);
    uint64 numBlocks = endBlock - startBlock + 1;

    // TODO: no-read optimization of offs == 0

    for(uint64 b = 0; b < numBlocks; b++) {
        uint64 blockID = startBlock + b;
        
        m_CacheLock.Spinlock();
        CachedBlock* cb;
        if(!GetCachedBlock(subID, blockID, GetBlockSize(subID), m_Cache, &cb)) {
            m_CacheLock.Unlock();
            cb->dataLock.Lock();

            Atomic<uint64> finished;
            Atomic<uint64> success;

            finished = 0;
            success = 0;

            ScheduleOperation(subID, blockID, 1, false, cb->data, &finished, &success);

            while(finished.Read() == 0) ;
                // TODO: Yield
        } else {
            m_CacheLock.Unlock();
            cb->dataLock.Lock();
        }

        uint64 offs = pos - (blockID * GetBlockSize(subID));
        uint64 rem = GetBlockSize(subID) - offs;
        if(rem > bufferSize)
            rem = bufferSize;

        if(!kmemcpy_usersafe(cb->data + offs, realBuffer, rem)) {
            cb->dataLock.Unlock();
            m_CacheLock.Spinlock();
            ReleaseCachedBlock(cb, m_Cache);
            m_CacheLock.Unlock();
            return ErrorInvalidBuffer;
        }
        cb->dataLock.Unlock();
        m_CacheLock.Spinlock();
        ReleaseCachedBlock(cb, m_Cache);
        m_CacheLock.Unlock();

        pos += rem;
        realBuffer += rem;
        bufferSize -= rem;
    }

    return 0;
}

static StickyLock g_DriverLock;
static uint64 g_DriverIDCounter = 0;
static ktl::AnchorList<DeviceDriver, &DeviceDriver::m_Anchor> g_Drivers;

uint64 DeviceDriverRegistry::RegisterDriver(DeviceDriver* driver) {
    g_DriverLock.Spinlock();
    uint64 res = g_DriverIDCounter++;
    g_Drivers.push_back(driver);
    g_DriverLock.Unlock();
    return res;
}
void DeviceDriverRegistry::UnregisterDriver(DeviceDriver* driver) {
    g_DriverLock.Spinlock();
    g_Drivers.erase(driver);
    g_DriverLock.Unlock();
}
DeviceDriver* DeviceDriverRegistry::GetDriver(uint64 id) {
    g_DriverLock.Spinlock();
    for(DeviceDriver& driver : g_Drivers) {
        if(driver.GetDriverID() == id) {
            g_DriverLock.Unlock();
            return &driver;
        }
    }
    g_DriverLock.Unlock();
    return nullptr;
}
DeviceDriver* DeviceDriverRegistry::GetDriver(const char* name) {
    g_DriverLock.Spinlock();
    for(DeviceDriver& driver : g_Drivers) {
        if(kstrcmp(driver.GetName(), name) == 0) {
            g_DriverLock.Unlock();
            return &driver;
        }
    }
    g_DriverLock.Unlock();
    return nullptr;
}