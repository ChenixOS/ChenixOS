#include "QueueLock.h"

#include "task/Scheduler.h"

// 队列锁
// 通过维护一个等待队列来实现的公平锁
// 底层原子性基于StickyLock加锁实现

QueueLock::QueueLock()
    : m_Count(1)
{ }
QueueLock::QueueLock(uint64 count)
    : m_Count(count)
{ }

void QueueLock::Lock() {
    m_Lock.Spinlock();
    if(m_Count > 0) {
        m_Count--;
        m_Lock.Unlock();
    } else { // 锁已经被获取则将线程加入到队列中
        Scheduler::ThreadSetSticky();

        auto tInfo = Scheduler::GetCurrentThreadInfo();

        QueueLockEntry entry;
        entry.thread = tInfo;
        m_Queue.push_back(&entry);
        m_Lock.Unlock();

        // no need to check for error, QUEUE_LOCK is uninterruptible
        Scheduler::ThreadBlock(ThreadState::QUEUE_LOCK, 0);

        Scheduler::ThreadUnsetSticky();
    }
}

void QueueLock::Unlock() {
    m_Lock.Spinlock();
    if(m_Queue.empty()) {
        m_Count++;
        m_Lock.Unlock();
    } else { // 退出锁 将下一个线程恢复到状态(既默认获得了锁)
        QueueLockEntry& e = m_Queue.front();
        m_Queue.pop_front();
        e.thread->registers.rax = 0;
        e.thread->state.type = ThreadState::READY;
        m_Lock.Unlock();
    }
}