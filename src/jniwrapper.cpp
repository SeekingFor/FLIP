#include "jniwrapper.h"
#include "global.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <dlib/threads.h>

#ifdef JNI_SUPPORT

int main(int argc, char **argv);

void mainwrapper(void *param)
{
#ifdef _WIN32
	_mkdir("FLIP");
#else
	mkdir("FLIP",0777);
#endif
	global::basepath="FLIP/";
	char *args[]={"FLIP"};
	main(1,args);
}

extern "C" JNIEXPORT void JNICALL Java_plugins_FLIP_FLIPPlugin_StartFLIP(JNIEnv *env, jobject obj)
{
	global::shutdown=false;
	dlib::threads_kernel_shared_helpers::spawn_thread(mainwrapper,0);
	return;
}

extern "C" JNIEXPORT void JNICALL Java_plugins_FLIP_FLIPPlugin_StopFLIP(JNIEnv *env, jobject obj)
{
	global::shutdown=true;
	return;
}

extern "C" JNIEXPORT jstring JNICALL Java_plugins_FLIP_FLIPPlugin_VersionString(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF(FLIP_VERSION);
}

extern "C" JNIEXPORT jlong JNICALL Java_plugins_FLIP_FLIPPlugin_VersionLong(JNIEnv *env, jobject obj)
{
	return FLIP_VERSION_LONG;
}

#endif	// JNI_SUPPORT
