#ifndef _DALVIK_H_
#define _DALVIK_H_

#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdint.h>    /* C99 */

#include "../common.h"
#include "object.h"

#ifdef HAVE_STDINT_H
# include <stdint.h>    /* C99 */
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;
#else
typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long long u8;
typedef signed char s1;
typedef signed short s2;
typedef signed int s4;
typedef signed long long s8;
#endif


typedef struct DalvikNativeMethod_t {
    const char *name;
    const char *signature;
    DalvikNativeFunc fnPtr;
} DalvikNativeMethod;

/* flags for dvmMalloc */
enum {
    ALLOC_DEFAULT = 0x00, ALLOC_DONT_TRACK = 0x01, /* don't add to internal tracking list */
    ALLOC_NON_MOVING = 0x02,
};

//typedef void* (*dvmIsStaticMethod_func)(void*);

typedef int (*dvmComputeMethodArgsSize_func)(void *);

typedef void (*dvmCallMethod_func)(void *, const Method *, void *, void *, void *, ...);

typedef size_t (*dexProtoGetParameterCount_func)(const DexProto *);

typedef ArrayObject *(*dvmAllocArrayByClass_func)(void *, size_t, int);

typedef void *(*dvmBoxPrimitive_func)(jvalue, void *);

typedef void *(*dvmFindPrimitiveClass_func)(const char);

typedef void (*dvmReleaseTrackedAlloc_func)(void *, void *);

typedef ClassObject *(*dvmFindArrayClass_func)(const char *, void *);

//typedef jlong (*dvmGetArgLong_func)(const u4*,int);
typedef int (*dvmCheckException_func)(void *);

typedef Object*(*dvmInvokeMethod_func)(Object* invokeObj, const Method* meth, ArrayObject* argList, ArrayObject* params, ClassObject* returnType, bool noAccessCheck);

typedef ClassObject* (*dvmFindSystemClass_func)(const char* descriptor);
typedef jclass (*dvmFindJNIClass_func)(JNIEnv *env,const char *classDesc);
typedef char* (*dvmDescriptorToName_func)(const char* str);
typedef JNIEnv * (*AndroidRuntime_getJNIEnv_func)();
typedef void *(*dvmGetMethodFromReflect_func)(void *);

typedef Object *(*dvmGetException_func)(void *);

typedef Object *(*dvmCreateReflectMethodObject_func)(const Method *);

typedef ClassObject *(*dvmGetBoxedReturnType_func)(const Method *);

//typedef int (*dvmIsPrimitiveClass_func)(ClassObject*);
typedef int (*dvmUnboxPrimitive_func)(void *, ClassObject *, void *);

typedef Object *(*dvmDecodeIndirectRef_func)(void *self, jobject jobj);

typedef void *(*dvmThreadSelf_func)();

typedef ClassObject *(*dvmResolveClass_func)(const ClassObject *referrer, u4 classIdx,
                                             bool fromUnverifiedConstant);

#endif