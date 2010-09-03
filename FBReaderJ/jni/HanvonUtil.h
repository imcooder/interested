
#ifndef HANVONUTIL_H_
#define HANVONUTIL_H_


#ifdef DEBUG_LOG
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,	"=== HAN.WANG ===",	__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,	"=== HAN.WANG ===",	__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,		"=== HAN.WANG ===",	__VA_ARGS__)
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE,	"=== HAN.WANG ===",	__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,		"=== HAN.WANG ===",	__VA_ARGS__)

#else
#define LOGD(...)
#define LOGE(...)
#define LOGI(...)
#define LOGV(...)
#define LOGW(...)

#endif

#endif /* HANVONUTIL_H_ */
