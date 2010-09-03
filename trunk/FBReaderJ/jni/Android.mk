LOCAL_PATH := $(call my-dir)
#############################################
# 调试信息： TRUE=打印日志
DEBUG := true
LDDEBUG :=
DEBUG_FLAG :=

$(info ============================================)
DMODE := $(shell echo $(DEBUG) | tr a-z A-Z)
ifeq ($(DMODE),TRUE)
  $(eval LDDEBUG := -llog)
  $(eval DEBUG_FLAG := -DDEBUG_LOG)
  $(info LOG_MODE = debug)
else
  $(info LOG_MODE = release)
endif
$(info ============================================)
COMMON_CFLAGS := -fsigned-char $(DEBUG_FLAG) 
#############################################
# 
include $(CLEAR_VARS)

LOCAL_MODULE                  := DeflatingDecompressor
LOCAL_SRC_FILES               := DeflatingDecompressor/DeflatingDecompressor.cpp
LOCAL_LDLIBS                  := -lz


LOCAL_CFLAGS += $(COMMON_CFLAGS)
LOCAL_LDLIBS += $(LDDEBUG)

include $(BUILD_SHARED_LIBRARY)

#############################################
# 
include $(CLEAR_VARS)

LOCAL_MODULE                  := LineBreak
LOCAL_SRC_FILES               := LineBreak/LineBreaker.cpp LineBreak/liblinebreak-2.0/linebreak.c LineBreak/liblinebreak-2.0/linebreakdata.c LineBreak/liblinebreak-2.0/linebreakdef.c


LOCAL_CFLAGS += $(COMMON_CFLAGS)
LOCAL_LDLIBS += $(LDDEBUG)

include $(BUILD_SHARED_LIBRARY)

#############################################
# 
include $(CLEAR_VARS)

LOCAL_MODULE                  := TxtParser
LOCAL_SRC_FILES               := TxtParser/TxtParser.cpp


LOCAL_CFLAGS += $(COMMON_CFLAGS)
LOCAL_LDLIBS += $(LDDEBUG)

include $(BUILD_SHARED_LIBRARY)
