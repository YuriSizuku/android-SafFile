#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <assert.h>
#include <android/log.h>
#include "xhook.h"
#include "safhook.h"

#define TAG "#YURIC"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define CLASSPATH "xxx/xxx/xxx"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;
    LOGI("libyurihook JNI_OnLoad start!\n");

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGI("libyurihook JNI_OnLoad! GetEnv failed");
		return -1;
    }
	
    assert(env != NULL);
    //jni register
    jclass clazz = (*env)->FindClass(env, CLASSPATH);
    if(registerSafJni(vm, env, clazz)<0)
    {
        LOGE("RegisterNatives error!");
        return JNI_ERR;
    }    
    
    // do not hook here because that other library may not loaded
    result = JNI_VERSION_1_4;
    LOGI("libyurihook JNI_OnLoad! finished!\n");
    return result;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved){
    LOGI("libyurihook JNI_OnUnload");
}