#include "art_5_0.h"
#include "art_5_1.h"
#include "art_6_0.h"
#include "../common.h"

extern "C" void art_quick_dexposed_invoke_handler();


/*6.0兼容方案=====================================================================================================================*/

void art_dispatch_6_0(JNIEnv *env, jobject src, jobject dest, bool javaBridge) {
    art_6_0::mirror::ArtMethod *smeth =
            (art_6_0::mirror::ArtMethod *) env->FromReflectedMethod(src);

    art_6_0::mirror::ArtMethod *dmeth =
            (art_6_0::mirror::ArtMethod *) env->FromReflectedMethod(dest);

    smeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_ = reinterpret_cast<void *>(art_quick_dexposed_invoke_handler);

}

extern "C" uint64_t artQuickDexposedInvokeHandler_6_0(art_6_0::mirror::ArtMethod *proxy_method,
                                                      art_6_0::mirror::Object *receiver, void *self,
                                                      void *sp)
/*__attribute__ ((shared_locks_required(Locks::mutator_lock_)))*/
{
#ifdef DEBUG
    LOGD("artQuickDexposedInvokeHandler_6_0");
#endif
    // 解出参数
    return 0;
}


/*5.1兼容方案=====================================================================================================================*/

void art_dispatch_5_1(JNIEnv *env, jobject src, jobject dest, bool javaBridge) {
    art_5_1::mirror::ArtMethod *smeth =
            (art_5_1::mirror::ArtMethod *) env->FromReflectedMethod(src);

    art_5_1::mirror::ArtMethod *dmeth =
            (art_5_1::mirror::ArtMethod *) env->FromReflectedMethod(dest);

    smeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_ = reinterpret_cast<void *>(art_quick_dexposed_invoke_handler);

}

extern "C" uint64_t artQuickDexposedInvokeHandler_5_1(art_5_1::mirror::ArtMethod *proxy_method,
                                                      art_5_1::mirror::Object *receiver, void *self,
                                                      void *sp)
/*__attribute__ ((shared_locks_required(Locks::mutator_lock_)))*/
{
#ifdef DEBUG
    LOGD("artQuickDexposedInvokeHandler_5_1");
#endif

    return 0;
}


/*5.0兼容方案=====================================================================================================================*/

void art_dispatch_5_0(JNIEnv *env, jobject src, jobject dest, bool javaBridge) {
    art_5_0::mirror::ArtMethod *smeth =
            (art_5_0::mirror::ArtMethod *) env->FromReflectedMethod(src);

    art_5_0::mirror::ArtMethod *dmeth =
            (art_5_0::mirror::ArtMethod *) env->FromReflectedMethod(dest);

    smeth->entry_point_from_compiled_code_ = reinterpret_cast<void *>(art_quick_dexposed_invoke_handler);

}

extern "C" uint64_t artQuickDexposedInvokeHandler_5_0(art_5_0::mirror::ArtMethod *proxy_method,
                                                      art_5_0::mirror::Object *receiver, void *self,
                                                      void *sp)
/*__attribute__ ((shared_locks_required(Locks::mutator_lock_)))*/
{
#ifdef DEBUG
    LOGD("artQuickDexposedInvokeHandler_5_0");
#endif

    return 0;
}