#ifndef _DALVIK_CORE_H_
#define _DALVIK_CORE_H_

#include <string>
#include "dalvik.h"

/**
 * "dalvik/vm/oo/Class.h"
 *
 * Compute the number of argument words (u4 units) required by the
 * given method's prototype. For example, if the method descriptor is
 * "(IJ)D", this would return 3 (one for the int, two for the long;
 * return value isn't relevant).
 *
 *
 * int dvmComputeMethodArgsSize(void *)
 */
extern dvmComputeMethodArgsSize_func dvmComputeMethodArgsSize_fnPtr;

//dvmIsStaticMethod_func dvmIsStaticMethod_fnPtr;

/*
 * "dalvik/vm/interp/Stack.h"
 *
 * Call an interpreted method from native code.  If this is being called
 * from a JNI function, references in the argument list will be converted
 * back to pointers.
 *
 * "obj" should be NULL for "direct" methods.
 *
 *
 * void dvmCallMethod(Thread* self, const Method* method, Object* obj, JValue* pResult, ...);
 */
extern dvmCallMethod_func dvmCallMethod_fnPtr;

/*
 * "dalvik/libdex/DexProto.h"
 *
 * Get the parameter count of the given prototype.
 *
 *
 * size_t dexProtoGetParameterCount(const DexProto* pProto);
 */
extern dexProtoGetParameterCount_func dexProtoGetParameterCount_fnPtr;

/*
 * "dalvik/vm/oo/Array.h"
 *
 * Create a new array, given an array class.  The class may represent an
 * array of references or primitives.
 *
 * Returns NULL with an exception raised if allocation fails.
 *
 *
 * extern "C" ArrayObject* dvmAllocArrayByClass(ClassObject* arrayClass, size_t length, int allocFlags);
 */
extern dvmAllocArrayByClass_func dvmAllocArrayByClass_fnPtr;

/*
 * "dalvik/vm/reflect/Reflect.h"
 *
 * Box a primitive value into an object.  If "returnType" is
 * not primitive, this just returns "value" cast to an object.
 *
 *
 * DataObject* dvmBoxPrimitive(JValue value, ClassObject* returnType);
 */
extern dvmBoxPrimitive_func dvmBoxPrimitive_fnPtr;

/*
 * "dalvik/vm/oo/Class.h"
 *
 * Find the class object representing the primitive type with the
 * given descriptor. This returns NULL if the given type character
 * is invalid.
 *
 *
 * ClassObject* dvmFindPrimitiveClass(char type);
 */
extern dvmFindPrimitiveClass_func dvmFindPrimitiveClass_fnPtr;

extern dvmReleaseTrackedAlloc_func dvmReleaseTrackedAlloc_fnPtr;
/*
 * "dalvik/vm/oo/Array.h"
 *
 * Find a matching array class.  If it doesn't exist, create it.
 *
 * "descriptor" looks like "[I".
 *
 * "loader" should be the defining class loader for the elements held
 * in the array.
 *
 *
 * ClassObject* dvmFindArrayClass(const char* descriptor, Object* loader);
 */
extern dvmFindArrayClass_func dvmFindArrayClass_fnPtr;

//dvmGetArgLong_func dvmGetArgLong_fnPtr;

extern dvmCheckException_func dvmCheckException_fnPtr;

extern dvmGetException_func dvmGetException_fnPtr;

extern dvmCreateReflectMethodObject_func dvmCreateReflectMethodObject_fnPtr;

extern dvmGetBoxedReturnType_func dvmGetBoxedReturnType_fnPtr;

//dvmIsPrimitiveClass_func dvmIsPrimitiveClass_fnPtr;

extern dvmUnboxPrimitive_func dvmUnboxPrimitive_fnPtr;

/*
 * Convert an indirect reference to an Object reference.  The indirect
 * reference may be local, global, or weak-global.
 *
 * If "jobj" is NULL, or is a weak global reference whose reference has
 * been cleared, this returns NULL.  If jobj is an invalid indirect
 * reference, kInvalidIndirectRefObject is returned.
 *
 * Note "env" may be NULL when decoding global references.
 *
 *
 * Object* dvmDecodeIndirectRef(Thread* self, jobject jobj)
 */
extern dvmDecodeIndirectRef_func dvmDecodeIndirectRef_fnPtr;

extern dvmThreadSelf_func dvmThreadSelf_fnPtr;

extern JNIEnv *jni_env;

extern ClassObject *classJavaLangObjectArray;
/**
 * NullPointerException
 */
extern jclass NPEClazz;
/**
 * ClassCastException
 */
extern jclass CastEClazz;
/**
 * Method.invoke(Object receiver, Object... args)
 */
extern jmethodID jInvokeMethod;
/**
 * Method.getDeclaringClass()
 */
extern jmethodID jClassMethod;

extern jint dalvik_setup(JNIEnv *env, int apilevel);

extern void dalvik_replace(JNIEnv *env, jobject src, jobject dest);

extern void dalvik_dispatch(JNIEnv *env, jobject src, jobject dest, bool javaBridge, const char*lpszMethodDesc);

extern bool dalvik_is_dispatched(JNIEnv *env, jobject src);

static void dispatcher_cpp(const u4 *args, jvalue *pResult, const Method *method, void *self);
static void nativeFunc_dispatcher_only_log_call(const u4 *args, jvalue *pResult, const Method *method, void *self);

static void dispatcher_java(const u4 *args, jvalue *pResult, const Method *method, void *self);

extern bool dvmIsStaticMethod(const Method *method);

extern bool dvmIsPrimitiveClass(const ClassObject *clazz);

extern ArrayObject *boxMethodArgs(const Method *method, const u4 *args);

extern Method* bridgeHandleMethod;

extern jclass bridgeHandleClass;

class HotFixInfo {

public:
    Method *srcCopy;

    Method *dest;

	std::string sClassName;
	std::string sMethodName;
	std::string sMethodDesc;

    HotFixInfo(Method *srcCopy, Method *dest) {
        this->srcCopy = srcCopy;
        this->dest = dest;
    }

    ~HotFixInfo() {
        //todo
		//delete srcCopy;
    }
};
#endif