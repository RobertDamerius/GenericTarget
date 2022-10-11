#include <GenericTarget/TargetTime.hpp>
using namespace gt;


TimeInfo::TimeInfo(){
    nanoseconds = 0;
    second = 0;
    minute = 0;
    hour = 0;
    mday = 1;
    mon = 0;
    year = 0;
    wday = 1;
    yday = 0;
    isdst = -1;
}

TargetTime::TargetTime(){
    hardware = 0.0;
    ticks = 0;
    software = 0.0;
    unixTime = 0.0;
    numCPUOverloads = 0;
    numLostTicks = 0;
}

