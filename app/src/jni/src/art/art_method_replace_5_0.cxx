#include <time.h>

#include "art.h"
#include "art_5_0.h"
#include "../common.h"

void replace_5_0(JNIEnv *env, jobject src, jobject dest) {
    art_5_0::mirror::ArtMethod *smeth =
            (art_5_0::mirror::ArtMethod *) env->FromReflectedMethod(src);

    art_5_0::mirror::ArtMethod *dmeth =
            (art_5_0::mirror::ArtMethod *) env->FromReflectedMethod(dest);

    dmeth->declaring_class_->class_loader_ =
            smeth->declaring_class_->class_loader_; //for plugin classloader
    dmeth->declaring_class_->clinit_thread_id_ =
            smeth->declaring_class_->clinit_thread_id_;
    dmeth->declaring_class_->status_ = (void *)((char *)smeth->declaring_class_->status_ - 1);

    smeth->declaring_class_ = dmeth->declaring_class_;
    smeth->access_flags_ = dmeth->access_flags_;
    smeth->frame_size_in_bytes_ = dmeth->frame_size_in_bytes_;
    smeth->dex_cache_initialized_static_storage_ =
            dmeth->dex_cache_initialized_static_storage_;
    smeth->dex_cache_resolved_types_ = dmeth->dex_cache_resolved_types_;
    smeth->dex_cache_resolved_methods_ = dmeth->dex_cache_resolved_methods_;
    smeth->vmap_table_ = dmeth->vmap_table_;
    smeth->core_spill_mask_ = dmeth->core_spill_mask_;
    smeth->fp_spill_mask_ = dmeth->fp_spill_mask_;
    smeth->mapping_table_ = dmeth->mapping_table_;
    smeth->code_item_offset_ = dmeth->code_item_offset_;
    smeth->entry_point_from_compiled_code_ =
            dmeth->entry_point_from_compiled_code_;

    smeth->entry_point_from_interpreter_ = dmeth->entry_point_from_interpreter_;
    smeth->native_method_ = dmeth->native_method_;
    smeth->method_index_ = dmeth->method_index_;
    smeth->method_dex_index_ = dmeth->method_dex_index_;

    LOGD("replace_5_0: %p , %p", smeth->entry_point_from_compiled_code_,
         dmeth->entry_point_from_compiled_code_);

}

void setFieldFlag_5_0(JNIEnv *env, jobject field) {
    art_5_0::mirror::ArtField *artField =
            (art_5_0::mirror::ArtField *) env->FromReflectedField(field);
    artField->access_flags_ = artField->access_flags_ & (~0x0002) | 0x0001;
    LOGD("setFieldFlag_5_0: %d ", artField->access_flags_);
}