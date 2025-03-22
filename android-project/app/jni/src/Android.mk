LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := main

DUCKER_SRC_PATH := $(LOCAL_PATH)/../../../../src

# Add your application source files here...
LOCAL_SRC_FILES := \
  $(subst $(LOCAL_PATH)/,, \
    $(wildcard $(DUCKER_SRC_PATH)/*.cpp) \
  )

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(SDL_PATH)/include \
                    $(DUCKER_SRC_PATH)

LOCAL_SHARED_LIBRARIES := SDL3

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid  # SDL

include $(BUILD_SHARED_LIBRARY)
