#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <error.h>
#include <errno.h> //x86_64  arm64 errno in this 
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>
#include "xhook.h"
#include "safhook.h"
static JavaVM* g_vm;
static jclass g_javaClass = NULL;
static jmethodID g_javaGetFD = NULL;
static jmethodID g_javaMkdir = NULL;
static jmethodID g_javaRemove = NULL;
static JNINativeMethod g_Methods[] = {
        {"nativeHookFile","(Ljava/lang/String;Ljava/lang/String;)V", (void*)nativeHookFile},
        {"nativeInitSafJavaCallbacks", "()V", (void*)nativeInitSafJavaCallbacks},
};

#ifndef TAG
#define TAG "#YURIC"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#endif
#define PATH_MAX_LEN 256

char* jstr2cstr(JNIEnv* env, jstring jstr) 
{
	char* rtn = NULL;
 
	jclass clsstring = (*env)->FindClass(env, "java/lang/String");
	jstring strencode = (*env)->NewStringUTF(env, "utf-8"); // "GB2312"
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray) (*env)->CallObjectMethod(env, jstr, mid, strencode);
	jsize alen = (*env)->GetArrayLength(env, barr);
	jbyte* ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
	if (alen > 0) {
		rtn = (char *)malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	(*env)->ReleaseByteArrayElements(env, barr, ba, 0);
	return rtn;
}

FILE *fopen_saf(const char *pathname, const char *mode)
{
    FILE* fp=NULL;
    JNIEnv* env = NULL;
    (*g_vm)->AttachCurrentThread(g_vm, &env, NULL);
    if(!env)
    {
        LOGE("fopen_asf, env AttachCurrentThread failed!\n");
        return fopen(pathname, mode);
    }
    int mode2=0;
    if(mode[0] == 'w') mode2=1;
    
    fp = fopen(pathname, mode);
    if(!(fp || mode2 == 0 || errno != EACCES))
    {
        char buf[PATH_MAX_LEN];
        getcwd(buf, PATH_MAX_LEN);
        //LOGI("before fopen(%s, %s), cwd=%s\n", pathname, mode, buf);    
        jstring s_pathname = (*env)->NewStringUTF(env, pathname);
        jstring s_curdir = (*env)->NewStringUTF(env, buf);
        
        int fd = (*env)->CallStaticIntMethod(env, g_javaClass, g_javaGetFD, s_curdir, s_pathname, mode2 );
        (*env)->DeleteLocalRef(env, s_curdir);
        (*env)->DeleteLocalRef(env, s_pathname);
        fp = fdopen(fd, mode);
        //LOGI("after fopen_saf(%s, %s),fp=%x, cwd=%s\n", pathname, mode, (unsigned int)fp,buf);
    }
    return fp;
}

int mkdir_saf(const char *pathname, mode_t mode)
{
    int ret=mkdir(pathname, mode);
    if(ret==0) return ret;

    JNIEnv* env = NULL;
    (*g_vm)->AttachCurrentThread(g_vm, &env, NULL);
    if(!env)
    {
        LOGE("mkdir_saf, env AttachCurrentThread failed!\n");
        return ret;
    }   
    

    //LOGI("before mkdir_saf(%s, %d)!\n", pathname, mode);
    char buf[PATH_MAX_LEN];
    getcwd(buf, PATH_MAX_LEN);
    jstring s_pathname = (*env)->NewStringUTF(env, pathname);
    jstring s_curdir = (*env)->NewStringUTF(env, buf);
    
    ret = (*env)->CallStaticIntMethod(env, g_javaClass, g_javaMkdir, s_curdir, s_pathname, mode);
    (*env)->DeleteLocalRef(env, s_curdir);
    (*env)->DeleteLocalRef(env, s_pathname);
    return ret;
}

int remove_saf(const char *pathname){

    int ret=remove(pathname);
    if(ret==0) return ret;

    JNIEnv* env = NULL;
    (*g_vm)->AttachCurrentThread(g_vm, &env, NULL);
    if(!env)
    {
        LOGE("remove_saf, env AttachCurrentThread failed!\n");
        return remove(pathname);
    }

    //LOGI("beforre remove_saf(%s)", pathname);
    char buf[PATH_MAX_LEN];
    getcwd(buf, PATH_MAX_LEN);
    jstring s_pathname = (*env)->NewStringUTF(env, pathname);
    jstring s_curdir = (*env)->NewStringUTF(env, buf);
    
     ret = (*env)->CallStaticIntMethod(env, g_javaClass, g_javaRemove, s_curdir, s_pathname);
    (*env)->DeleteLocalRef(env, s_curdir);
    (*env)->DeleteLocalRef(env, s_pathname);
    return remove(pathname);
}

void nativeInitSafJavaCallbacks(JNIEnv* env, jclass clazz)
{
    LOGI("In nativeInitSafJavaCallbacks start!");
    g_javaGetFD=(*env)->GetStaticMethodID(env, clazz, "getFD", "(Ljava/lang/String;Ljava/lang/String;I)I");
    g_javaMkdir=(*env)->GetStaticMethodID(env, clazz, "mkdir", "(Ljava/lang/String;Ljava/lang/String;I)I");
    g_javaRemove = (*env)->GetStaticMethodID(env, clazz, "remove", "(Ljava/lang/String;Ljava/lang/String;)I");
    LOGI("In nativeInitSafJavaCallbacks finished!");
}

void nativeHookFile(JNIEnv* env, jclass clazz, jstring hooksoStr, jstring soPath)
{
    char buf[100];
    char *cstr_hooksoStr = jstr2cstr(env, hooksoStr);
    LOGI("nativeHookFile, %s \n", cstr_hooksoStr);
    char *cstr_soPath = jstr2cstr(env, soPath);
    if(cstr_soPath && strlen(cstr_soPath))
    {
        if (!dlopen(cstr_soPath, RTLD_LAZY)) //dlopen in advance
        LOGE("dlopen(%s,%d) error!\n", cstr_soPath,RTLD_LAZY);
        else LOGI("dlopen(%s,%d) success !\n", cstr_soPath,RTLD_LAZY);
    }
    
    if (xhook_register(cstr_hooksoStr, "fopen", fopen_saf, NULL))
        LOGE("xhook fopen register failed!");
    else LOGI("xhook fopen register successed!");
    
    if (xhook_register(cstr_hooksoStr, "mkdir", mkdir_saf, NULL))
        LOGE("xhook mkdir register failed!\n");
    else LOGI("xhook mkdir register successed!");
    
    if (xhook_register(cstr_hooksoStr, "remove", remove_saf, NULL))
        LOGE("xhook remove register failed!\n");
    else LOGI("xhook remove register successed!");
    
    xhook_refresh(0);
    free(cstr_hooksoStr);
    LOGI("nativeHookFile xhook finished!");
    if(cstr_soPath) free(cstr_soPath);
}

int registerSafJni(JavaVM* vm, JNIEnv* env, jclass clazz)
{
    LOGI("In registerSafJni start!");
    int ret;
    g_vm = vm;
    g_javaClass = (*env) -> NewGlobalRef(env, clazz);
    ret = (*env)->RegisterNatives(env, g_javaClass, 
          g_Methods, sizeof(g_Methods)/sizeof((g_Methods)[0]));
    LOGI("In registerSafJni finished!");
    return ret;
}