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

TimeInfo::TimeInfo(const TimeInfo& time){
    this->nanoseconds = time.nanoseconds;
    this->second = time.second;
    this->minute = time.minute;
    this->hour = time.hour;
    this->mday = time.mday;
    this->mon = time.mon;
    this->year = time.year;
    this->wday = time.wday;
    this->yday = time.yday;
    this->isdst = time.isdst;
}

TimeInfo::~TimeInfo(){}

TimeInfo& TimeInfo::operator=(const TimeInfo& rhs){
    this->nanoseconds = rhs.nanoseconds;
    this->second = rhs.second;
    this->minute = rhs.minute;
    this->hour = rhs.hour;
    this->mday = rhs.mday;
    this->mon = rhs.mon;
    this->year = rhs.year;
    this->wday = rhs.wday;
    this->yday = rhs.yday;
    this->isdst = rhs.isdst;
    return *this;
}

TargetTime::TargetTime(){
    hardware = 0.0;
    ticks = 0;
    software = 0.0;
    unixTime = 0.0;
}

TargetTime::TargetTime(const TargetTime& time){
    this->utc = time.utc;
    this->local = time.local;
    this->hardware = time.hardware;
    this->ticks = time.ticks;
    this->software = time.software;
    this->unixTime = time.unixTime;
}

TargetTime::~TargetTime(){}

TargetTime& TargetTime::operator=(const TargetTime& rhs){
    this->utc = rhs.utc;
    this->local = rhs.local;
    this->hardware = rhs.hardware;
    this->ticks = rhs.ticks;
    this->software = rhs.software;
    this->unixTime = rhs.unixTime;
    return *this;
}

