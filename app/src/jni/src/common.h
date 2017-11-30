#ifndef COMMON_H_
#define COMMON_H_

#include <jni.h>
#include <android/log.h>
#include "debug.h"

#define RETURN_BOOLEAN(_val)    do { pResult->i = (_val); return; } while(0)
#define RETURN_INT(_val)        do { pResult->i = (_val); return; } while(0)
#define RETURN_LONG(_val)       do { pResult->j = (_val); return; } while(0)
#define RETURN_FLOAT(_val)      do { pResult->f = (_val); return; } while(0)
#define RETURN_DOUBLE(_val)     do { pResult->d = (_val); return; } while(0)
#define RETURN_PTR(_val)        do { pResult->l = (Object*)(_val); return; } while(0)
#define RETURN_VOID()           do { pResult->i = 0xfefeabab; return; }while(0)


#endif /* COMMON_H_ */
