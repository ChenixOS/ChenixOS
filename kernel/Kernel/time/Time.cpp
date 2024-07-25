#include "Time.h"

#include "arch/MSR.h"
#include "arch/CPU.h"
#include "arch/Port.h"

#include "klib/stdio.h"

#include "syscalls/SyscallDefine.h"

namespace Time {

    static uint64 g_TSCTicksPerMilli;

    static void CalibrateTSC() {
        uint64 ticksPerMS = 0;
        for(int i = 0; i < 5; i++) {
            Port::OutByte(0x43, 0x30);

            constexpr uint16 pitTickInitCount = 39375; // 33 ms
            Port::OutByte(0x40, pitTickInitCount & 0xFF);
            Port::OutByte(0x40, (pitTickInitCount >> 8) & 0xFF);

            uint64 tscStart;
            uint64 edx, eax;
            __asm__ __volatile__ (
                "mfence;"
                "rdtsc;"
                "lfence"
                : "=d"(edx), "=a"(eax)
            );
            tscStart = (edx << 32) | eax;

            uint16 pitCurrentCount = 0;
            while(pitCurrentCount <= pitTickInitCount)
            {
                Port::OutByte(0x43, 0);
                pitCurrentCount = (uint16)Port::InByte(0x40) | ((uint16)Port::InByte(0x40) << 8);
            }

            uint64 tscEnd;
            __asm__ __volatile__ (
                "mfence;"
                "rdtsc;"
                "lfence"
                : "=d"(edx), "=a"(eax)
            );
            tscEnd = (edx << 32) | eax;

            uint64 elapsed = tscEnd - tscStart;
            ticksPerMS += elapsed / 33;
        }

        ticksPerMS /= 5;
        klog_info_isr("Time", "TSC runs at %i kHz", ticksPerMS);
        g_TSCTicksPerMilli = ticksPerMS;
    }

    bool Init() {
        uint64 eax, ebx, ecx, edx;
        CPU::CPUID(1, 0, eax, ebx, ecx, edx);
        if((edx & (1 << 4)) == 0) {
            klog_fatal("Time", "CPU Time stamp counter not supported");
            return false;
        }

        /*CPU::CPUID(0x80000001, 0, eax, ebx, ecx, edx);
        if((edx & (1 << 27)) == 0) {
            klog_fatal("Time", "RDTSCP instruction not supported");
            return false;
        }

        CPU::CPUID(0x80000007, 0, eax, ebx, ecx, edx);
        if((edx & (1 << 8)) == 0) {
            klog_fatal("Time", "TSC is not invariant");
            return false;
        }*/

        CalibrateTSC();

        return true;
    }

    uint64 GetTSCTicksPerMilli() {
        return g_TSCTicksPerMilli;
    }

    uint64 GetTSC() {
        uint64 edx, eax;
        __asm__ __volatile__ (
            "rdtsc"
            : "=d"(edx), "=a"(eax)
        );
        return ((edx << 32) & 0xFFFFFFFF00000000) | (eax & 0xFFFFFFFF);
    }

    // ==================================================
    // 下面是RTC的实现代码

    static uint8 ReadRTCReg(uint8 reg) {
        uint8 res;

        Port::OutByte(0x70, reg);
        res = Port::InByte(0x71);
        Port::OutByte(0x70,0x80);

        return res;
    }

    #define CONVERT_BCD(var) var = (var & 0x0F) + (var >> 4) * 10

    void GetRTC(DateTime* dt) {
        uint8 lastSecond = ReadRTCReg(0x00);
        uint8 lastMinute = ReadRTCReg(0x02);
        uint8 lastHour = ReadRTCReg(0x04);
        uint8 lastDOM = ReadRTCReg(0x07);
        uint8 lastMonth = ReadRTCReg(0x08);
        uint8 lastYear = ReadRTCReg(0x09);
        uint8 lastCentry = ReadRTCReg(0x32);

        while(true) {
            uint8 second = ReadRTCReg(0x00);
            uint8 minute = ReadRTCReg(0x02);
            uint8 hour = ReadRTCReg(0x04);
            uint8 dayOfMonth = ReadRTCReg(0x07);
            uint8 month = ReadRTCReg(0x08);
            uint8 year = ReadRTCReg(0x09);
            uint8 centry = ReadRTCReg(0x32);

            if(lastSecond == second && lastMinute == minute && lastHour == hour && lastDOM == dayOfMonth && lastMonth == month && lastYear == year) {
                break;
            }

            lastSecond = second;
            lastMinute = minute;
            lastHour = hour;
            lastDOM = dayOfMonth;
            lastMonth = month;
            lastYear = year;
            lastCentry = centry;
        }

        uint8 format = ReadRTCReg(0x0B);
        if((format & 0x04) == 0) {  // BCD mode
            CONVERT_BCD(lastSecond);
            CONVERT_BCD(lastMinute);
            CONVERT_BCD(lastHour);
            CONVERT_BCD(lastDOM);
            CONVERT_BCD(lastMonth);
            CONVERT_BCD(lastYear);
            CONVERT_BCD(lastCentry);
        }
        if((format & 0x02) == 0 && (lastHour & 0x80)) {
            lastHour = (lastHour & 0x7F) + 12;
        }

        dt->seconds = lastSecond;
        dt->minutes = lastMinute;
        dt->hours = lastHour;
        dt->dayOfMonth = lastDOM;
        dt->month = lastMonth;
        dt->year = lastYear;
        dt->fullYear = (lastCentry * 100) + lastYear;
    }

    static
    uint64 Date2timeStamp(int Year,
        int Mon,
        int Day,
        int Hour,
        int Min,
        int Second)
    {
        int TotalDay = 0;
        uint32 result = 0;

        // 按年计算天数
        for(int i = 1970;i < Year;i++) {
            if((i % 4 == 0 && i % 100 != 0) || i % 400 == 0) {
                TotalDay += 1;
            }
            TotalDay += 365;
        }

        // 计算月天数
        for(int i = 1;i < Mon;i++) {
            if(i == 2) {
                TotalDay += 28;
                if((Year % 4 == 0 && Year % 100 != 0) || Year % 400 == 0) {
                    TotalDay += 1;
                }
            } else if((i == 1) || (i == 3) || (i == 5) || (i == 7) || (i == 8) || (i == 10) || (i == 12)) {
                // 双数月
                TotalDay += 31;
            } else {
                TotalDay += 30;
            }
        }

        TotalDay += Day;
        TotalDay -= 1;

        // 总小时
        result = (TotalDay * 24) + Hour;
        // 总分钟
        result = (result * 60) + Min;
        // 总秒数
        result = (result * 60) + Second;

        return result;
    }

    // 计算出当前时间戳
    uint64 GetCurTimestamp() {
        DateTime t;
        GetRTC(&t);
        
        return Date2timeStamp(t.fullYear,t.month,t.dayOfMonth,t.hours,t.minutes,t.seconds);
    }

    SYSCALL_DEFINE0(syscall_gettime) {
        return GetCurTimestamp();
    }


}