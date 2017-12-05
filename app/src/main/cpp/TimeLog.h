#pragma once

#include <time.h>
#include <string>

using namespace std;


class CTimeLog {
public:
    CTimeLog(const char *sName);

    ~CTimeLog();

private:
    time_t m_timeBegin;
    std::string m_sName;
};