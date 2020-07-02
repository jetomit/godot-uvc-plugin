#include <jni.h>

#include "jni_common.h"
#include "libusb.h"
#include "libuvc/libuvc.h"

#undef JNI_PACKAGE_NAME
#define JNI_PACKAGE_NAME org_eyeskills_godot_plugin_android_uvc

#undef JNI_CLASS_NAME
#define JNI_CLASS_NAME UvcPlugin

static libusb_context* usb_ctx{nullptr};
static uvc_context* uvc_ctx{nullptr};

extern "C" {
JNIEXPORT jint JNICALL JNI_METHOD(foo)(JNIEnv* env, jobject object) {
	return 42;
}

JNIEXPORT jboolean JNICALL JNI_METHOD(init)(JNIEnv* env, jobject object) {
	if (!usb_ctx && libusb_init(&usb_ctx))
		return false;
	if (!uvc_ctx && uvc_init(&uvc_ctx, usb_ctx))
		return false;
	return true;
}

JNIEXPORT void JNICALL JNI_METHOD(finish)(JNIEnv* env, jobject object) {
	if (uvc_ctx) {
		uvc_exit(uvc_ctx);
		uvc_ctx = nullptr;
	}
	if (usb_ctx) {
		libusb_exit(usb_ctx);
		usb_ctx = nullptr;
	}
}

JNIEXPORT jboolean JNICALL JNI_METHOD(attached)(JNIEnv* env, jobject object, jint fd) {
	return false;
}

JNIEXPORT void JNICALL JNI_METHOD(detached)(JNIEnv* env, jobject object, jint fd) {
}
};
