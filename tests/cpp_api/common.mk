LOCAL_MODULE_TAGS := tests

LOCAL_CLANG := true
LOCAL_CFLAGS := -std=c++11 -Werror

intermediates := $(call intermediates-dir-for,STATIC_LIBRARIES,libRS,TARGET,)
LOCAL_C_INCLUDES += $(intermediates)
