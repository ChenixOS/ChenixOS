#include "StickyLock.h"

#include "task/Scheduler.h"

// 粘性锁
// 其实就是SpinLock回旋锁，非公平锁
// 既SpinLock+线程Yield+ThreadSetSticky的实现

StickyLock::StickyLock()
    : m_Val(0)
{ }

void StickyLock::Spinlock() {
    Scheduler::ThreadSetSticky();

    while(!TryLock()) ;
}
void StickyLock::Unlock() {
    DoUnlock();

    Scheduler::ThreadUnsetSticky();
}

void StickyLock::Spinlock_Cli() {
    Scheduler::ThreadDisableInterrupts();
    while(!TryLock()) ;
}
void StickyLock::Unlock_Cli() {
    DoUnlock();
    Scheduler::ThreadEnableInterrupts();
}

void StickyLock::Spinlock_Raw() {
    while(!TryLock()) ;
}
void StickyLock::Unlock_Raw() {
    DoUnlock();
}

bool StickyLock::TryLock() {
    uint64 val = 1;
    uint64* loc = &m_Val;
    __asm__ __volatile__ (
        "lock xchgq %0, (%1)"
        : "+r"(val), "+r"(loc)
    );
    return val == 0;
}
void StickyLock::DoUnlock() {
    uint64 val = 0;
    uint64* loc = &m_Val;
    __asm__ __volatile__ (
        "lock xchgq %0, (%1)"
        : "+r"(val), "+r"(loc)
    );
}