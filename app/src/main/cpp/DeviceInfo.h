#pragma once

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <android/log.h>
#include "debug.h"
#include <string>
using namespace std;

class CDeviceInfo {
public:
    static int isArt();
    static int getSdkInt();
    static std::string getOsReleaseVer();
    static std::string getMac();
    static int getAndroidId();
    static int getImei();

};

