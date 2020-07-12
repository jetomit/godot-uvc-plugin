#include <Defs.hpp>
#include <Godot.hpp>

#include "uvc-feed.h"
#include "uvc-server.h"

using namespace godot;

// GDNative entry points
extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	Godot::gdnative_init(o);
	Godot::print("godot_gdnative_init");
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	Godot::print("godot_gdnative_terminate");
	Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_gdnative_singleton() {
	Godot::print("godot_gdnative_singleton");
	static UvcServer* singleton = nullptr;
	if (!singleton)
		singleton = UvcServer::_new();
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	Godot::nativescript_init(handle);
	Godot::print("godot_nativescript_init");

	register_class<UvcServer>();
	register_class<CameraFeedUvc>();
}
