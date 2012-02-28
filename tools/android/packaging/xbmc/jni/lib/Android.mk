LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := xbmc
LOCAL_SRC_FILES := libxbmc.so
include $(PREBUILT_SHARED_LIBRARY)
