#include <jni.h>
#ifndef _ASFHOOK_H
#define _ASFHOOK_H
char* jstr2cstr(JNIEnv*, jstring); 
void nativeHookFile(JNIEnv*, jclass, jstring, jstring);
void nativeInitSafJavaCallbacks(JNIEnv*, jobject);
int registerSafJni(JavaVM*, JNIEnv*, jclass);
#endif