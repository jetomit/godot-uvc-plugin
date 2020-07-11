#include <sstream>
#include <map>

#include <libusb.h>
#include <libuvc/libuvc.h>
#include "libuvc/libuvc_internal.h"

#include <Engine.hpp>
#include <Dictionary.hpp>
#include <CameraServer.hpp>
#include <CameraFeed.hpp>
#include <Godot.hpp>
#include <Reference.hpp>

using namespace godot;

static libusb_context* usb_ctx{nullptr};
static uvc_context* uvc_ctx{nullptr};

/*
class CameraFeedAndroid : public CameraFeed {
	GDCLASS(CameraFeedAndroid, CameraFeed);

public:
	CameraFeedAndroid() {
	};

	bool activate_feed() {
		return true;
	}

	void deactivate_feed() {
	};


	// Can’t do this in ~CameraFeedAndroid because it might get called
	// much later, when the last Ref goes out of scope.

	// TODO make this private
	void frame_callback(uvc_frame *frame);

private:
	int fd;
	uvc_device_handle* uvc_devh;
	struct uvc_stream_ctrl* stream_ctrl;

};
*/

// Map file descriptors to associated feeds.
static std::map<int, Ref<CameraFeed>> feeds;

static Ref<CameraFeed> destroy(int fd) {
	auto p_feed = feeds.find(fd);
	if (p_feed == feeds.end())
		return {};
	Ref<CameraFeed> feed{p_feed->second};
	feeds.erase(p_feed);

	// Make sure we stop recording if we are.
//	if (feed->is_active())
//		feed->deactivate_feed();
//	::free(feed->stream_ctrl);
//	uvc_close(feed->uvc_devh);
//	feed->fd = -1;
	return feed;
}

static Ref<CameraFeed> create(int fd, String name) {
	Godot::print("create(fd=%d)");

	if (feeds.find(fd) != feeds.end())
		destroy(fd); // should have been destroyed already, do it now

	libusb_device_handle* usb_devh;
	int status = libusb_wrap_sys_device(usb_ctx, (intptr_t)fd, &usb_devh);
	if (status != 0) {
		Godot::print("libusb_wrap_sys_device(): %s (%d)");
		return {};
	}

	// libuvc init
	uvc_device* uvc_dev;
	uvc_dev = (uvc_device*)malloc(sizeof(uvc_device)); // XXX THIS LEAKS!!??
	uvc_dev->ctx = uvc_ctx;
	uvc_dev->ref = 0;
	uvc_dev->usb_dev = libusb_get_device(usb_devh);

	uvc_device_handle* uvc_devh;
	status = uvc_open_with_usb_devh(uvc_dev, &uvc_devh, usb_devh);
	if (status != 0) {
		Godot::print("uvc_open_with_usb_devh: %s (%d)");
		return {};
	}

	uvc_print_diag(uvc_devh, NULL);

	/* Try to negotiate a 160x120 14 fps YUYV stream profile */
	struct uvc_stream_ctrl* stream_ctrl =
		(struct uvc_stream_ctrl*)malloc(sizeof(struct uvc_stream_ctrl));
	status = uvc_get_stream_ctrl_format_size(
		uvc_devh, stream_ctrl, /* result stored in ctrl */
		UVC_FRAME_FORMAT_ANY, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
		160, 120, 15); /* width, height, fps */
	if (status != 0) {
		uvc_close(uvc_devh);
		Godot::print("uvc_get_stream_ctrl_format_size: %s (%d)");
		return {};
	}
	uvc_print_stream_ctrl(stream_ctrl, NULL);

	Ref<CameraFeed> feed{};
	feed.instance();
	//feed->set_name(name);
	//feed->fd = fd;
	//feed->uvc_devh = uvc_devh;
	//feed->stream_ctrl = stream_ctrl;

	feeds.insert({fd, feed});
	return feed;
}

class Simple : public Reference {
	GODOT_CLASS(Simple, Reference);
public:
	Simple() { }

	void _init() {
		Godot::print("Simple::_init()");

		if (!usb_ctx && libusb_init(&usb_ctx))
			return;
		if (!uvc_ctx && uvc_init(&uvc_ctx, usb_ctx))
			return;

		auto *p = godot::Engine::get_singleton();
		auto *cs = p->get_singleton("Uvc");
		if (!cs) {
			godot::Godot::print("couldn’t load Uvc plugin");
			return;
		}

		std::ostringstream ss;
		ss << "godot_gdnative_init get engine, e=" << p << ", some numbers: " << p->get_target_fps() << " " << p->get_frames_drawn() << " " << p->get_target_fps() << " this=" << this << " cs=" << cs;
		godot::Godot::print(ss.str().c_str());
		ss.clear();

		ss << "connect returns: " << (int)(cs->connect("connected", this, "connected"));
		godot::Godot::print(ss.str().c_str());
	}

	Variant connected(Variant fd) {
		std::ostringstream ss;
		ss << "Simple::connected(" << int(fd) << ")";
		godot::Godot::print(ss.str().c_str());

		auto *p = godot::Engine::get_singleton();
		auto *cs = Object::cast_to<CameraServer>(p->get_singleton("CameraServer"));
		if (cs) {
			Ref<CameraFeed> feed;
			feed.instance();
			cs->add_feed(feed);
		} else {
			godot::Godot::print("couldn’t load CameraServer");
		}

		Variant ret = fd;
		create(int(fd), "XYZZY");
		return ret;
	}

	Variant disconnected(Variant fd) {
		Variant ret = fd;
		return ret;
	}

	static void _register_methods() {
		register_method("connected", &Simple::connected);
		register_method("disconnected", &Simple::disconnected);

		/**
		 * The line below is equivalent to the following GDScript export:
		 *     export var _name = "Simple"
		 **/
		register_property<Simple, String>("base/name", &Simple::_name, String("Simple"));
		//register_property<CameraFeedAndroid, String>("base/name", &CameraFeedAndroid::_name, String("CameraFeedAndroid"));

		/** Alternatively, with getter and setter methods: */
		register_property<Simple, int>("base/value", &Simple::set_value, &Simple::get_value, 0);

		/** Registering a signal: **/
		// register_signal<Simple>("signal_name");
		// register_signal<Simple>("signal_name", "string_argument", GODOT_VARIANT_TYPE_STRING)
	}

	String _name;
	int _value;

	void set_value(int p_value) {
		_value = p_value;
	}

	int get_value() const {
		return _value;
	}
};

/** GDNative Initialize **/
extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	godot::Godot::gdnative_init(o);
	godot::Godot::print("godot_gdnative_init");
}

/** GDNative Terminate **/
extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	godot::Godot::print("godot_gdnative_terminate");
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_gdnative_singleton() {
	static Simple* singleton = nullptr;
	godot::Godot::print("godot_gdnative_singleton");
	if (!singleton)
		singleton = Simple::_new();
}

/** NativeScript Initialize **/
extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	godot::Godot::nativescript_init(handle);
	godot::Godot::print("godot_nativescript_init");

	godot::register_class<Simple>();
}
