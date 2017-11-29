#include "TimeLog.h"
#include "debug.h"


CTimeLog::CTimeLog(const char *sName) {
	time(&m_timeBegin);
	if (sName) {
		m_sName = sName;
	}
	LOGD("CTimeLog start: [%s]", this->m_sName.c_str());
}

CTimeLog::~CTimeLog() {
	time_t timeEnd;
	time(&timeEnd);
	LOGD("CTimeLog finished: [%s] time used: %.2lf s", this->m_sName.c_str(), difftime(timeEnd, m_timeBegin));
}