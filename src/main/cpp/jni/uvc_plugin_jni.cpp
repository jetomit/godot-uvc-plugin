#include <jni.h>

#include "jni_common.h"

#undef JNI_PACKAGE_NAME
#define JNI_PACKAGE_NAME org_eyeskills_godot_plugin_android_uvc

#undef JNI_CLASS_NAME
#define JNI_CLASS_NAME UvcPlugin

extern "C" {
JNIEXPORT jint JNICALL JNI_METHOD(foo)(JNIEnv* env, jobject object) {
	return 42;
}

JNIEXPORT jboolean JNICALL JNI_METHOD(attached)(JNIEnv* env, jobject object, jint fd) {
	return false;
}

JNIEXPORT void JNICALL JNI_METHOD(detached)(JNIEnv* env, jobject object, jint fd) {
}
};
