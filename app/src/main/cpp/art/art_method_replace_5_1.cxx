#include <time.h>

#include "art.h"
#include "art_5_1.h"
#include "../common.h"

void replace_5_1(JNIEnv *env, jobject src, jobject dest) {
    art_5_1::mirror::ArtMethod *smeth =
            (art_5_1::mirror::ArtMethod *) env->FromReflectedMethod(src);

    art_5_1::mirror::ArtMethod *dmeth =
            (art_5_1::mirror::ArtMethod *) env->FromReflectedMethod(dest);

    dmeth->declaring_class_->class_loader_ =
            smeth->declaring_class_->class_loader_; //for plugin classloader
    dmeth->declaring_class_->clinit_thread_id_ =
            smeth->declaring_class_->clinit_thread_id_;
    dmeth->declaring_class_->status_ = (void *) ((char *) smeth->declaring_class_->status_ - 1);

    smeth->declaring_class_ = dmeth->declaring_class_;
    smeth->dex_cache_resolved_types_ = dmeth->dex_cache_resolved_types_;
    smeth->access_flags_ = dmeth->access_flags_;
    smeth->dex_cache_resolved_methods_ = dmeth->dex_cache_resolved_methods_;
    smeth->dex_code_item_offset_ = dmeth->dex_code_item_offset_;
    smeth->method_index_ = dmeth->method_index_;
    smeth->dex_method_index_ = dmeth->dex_method_index_;

    smeth->ptr_sized_fields_.entry_point_from_interpreter_ =
            dmeth->ptr_sized_fields_.entry_point_from_interpreter_;

    smeth->ptr_sized_fields_.entry_point_from_jni_ =
            dmeth->ptr_sized_fields_.entry_point_from_jni_;
    smeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_ =
            dmeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_;

    LOGD("replace_5_1: %p , %p",
         smeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_,
         dmeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_);

}

void setFieldFlag_5_1(JNIEnv *env, jobject field) {
    art_5_1::mirror::ArtField *artField =
            (art_5_1::mirror::ArtField *) env->FromReflectedField(field);
    artField->access_flags_ = artField->access_flags_ & (~0x0002) | 0x0001;
    LOGD("setFieldFlag_5_1: %d ", artField->access_flags_);
}
