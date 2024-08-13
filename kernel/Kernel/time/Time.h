#pragma once

#include "types.h"

namespace Time {

    bool Init();

    uint64 GetTSCTicksPerMilli();
    uint64 GetTSC();

    struct DateTime {
        uint8 seconds;
        uint8 minutes;
        uint8 hours;
        uint8 dayOfMonth;
        uint8 month;
        uint8 year;
        uint16 fullYear;
    };

    void GetRTC(DateTime* dt);

    void Delay(uint64 ms);

}