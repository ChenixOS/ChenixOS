#pragma once
#include "Time.h"

#define WAIT_TIME(expr,ms) ({ for(uint64 t1 = Time::GetTSC(),t2 = 0;expr && (t2 - t1 <= (ms * Time::GetTSCTicksPerMilli()));t2 = Time::GetTSC()) {} })
