#ifndef _flip_jni_
#define _flip_jni_

#ifdef JNI_SUPPORT

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_plugins_FLIP_FLIPPlugin_StartFLIP(JNIEnv *env, jobject obj);
JNIEXPORT void JNICALL Java_plugins_FLIP_FLIPPlugin_StopFLIP(JNIEnv *env, jobject obj);
JNIEXPORT jstring JNICALL Java_plugins_FLIP_FLIPPlugin_VersionString(JNIEnv *env, jobject obj);
JNIEXPORT jlong JNICALL Java_plugins_FLIP_FLIPPlugin_VersionLong(JNIEnv *env, jobject obj);

#ifdef __cplusplus
}
#endif

#endif	// JNI_SUPPORT

#endif	// _flip_jni_
