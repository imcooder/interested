
#ifndef HW_TXTPARSER_H
#define HW_TXTPARSER_H


#include <jni.h>



#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_org_parser_txt_TxtParser_Parser(JNIEnv *env, jobject thiz, jobject thisParser,jstring strPath);

#ifdef __cplusplus
}
#endif

#endif
