
#include "DeviceInfo.h"
#include <time.h>
#include <dlfcn.h>
#include <errno.h>
#include <string>
#include "debug.h"
#include <sys/stat.h>
#include <include/cutils/properties.h>

int CDeviceInfo::getSdkInt() {
    char szBuff[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.build.version.sdk", szBuff, "0");
    int sdkInt = atoi(szBuff);
    return sdkInt;
}

std::string CDeviceInfo::getOsReleaseVer() {
    char szBuff[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.build.version.release", szBuff, "0");
    return szBuff;
}