#include <map>

#include <libusb.h>
#include <libuvc/libuvc.h>
#include "libuvc/libuvc_internal.h"

#include <Engine.hpp>
#include <Defs.hpp>
#include <Dictionary.hpp>
#include <CameraServer.hpp>
#include <CameraFeed.hpp>
#include <Godot.hpp>
#include <Reference.hpp>

using namespace godot;

static libusb_context* usb_ctx{nullptr};
static uvc_context* uvc_ctx{nullptr};

static const char* libuvc_error_name (int e) {
	return uvc_strerror((uvc_error_t)e);
}

extern "C" void uvc_callback(struct uvc_frame* frame, void* userptr);

class CameraFeedUvc : public CameraFeed {
	GODOT_CLASS(CameraFeedUvc, CameraFeed);
public:
	CameraFeedUvc() { };

	void _init() {
		Godot::print("CameraFeedUvc::_init()");
	}

	// XXX without copying this from CameraFeed, base class
	// (de)activate_feed methods get called even for CameraFeedUvc
	bool is_active;
	void set_active(bool p_is_active) {
		Godot::print("CameraFeedUvc::set_active()");
		//CameraFeed::set_active(p_is_active);
		if (p_is_active == is_active) {
			// all good
		} else if (p_is_active) {
			// attempt to activate this feed
			if (activate_feed()) {
				Godot::print("Activate ");
				is_active = true;
			}
		} else {
			// just deactivate it
			deactivate_feed();
			Godot::print("Deactivate ");
			is_active = false;
		}
	}

	bool activate_feed() {
		Godot::print("CameraFeedUvc::activate_feed()");
		if (fd < 0) {
			Godot::print("fd < 0, bailing!");
			return false;
		}
		int status = uvc_start_streaming(uvc_devh, stream_ctrl, uvc_callback, this, 0);
		if (status != 0) {
			Godot::print(String{"uvc_start_streaming: "} + String{libuvc_error_name(status)});
			return false;
		}
		return true;
	}

	void deactivate_feed() {
		Godot::print("CameraFeedUvc::deactivate_feed()");
	};

	static void _register_methods() {
		register_method("activate_feed", &CameraFeedUvc::activate_feed);
		register_method("deactivate_feed", &CameraFeedUvc::deactivate_feed);
		register_method("set_active", &CameraFeedUvc::set_active);
	}

	// TODO make this private
//private:
	// This callback function runs once per frame. Use it to
	// perform any quick processing you need, or have it put the
	// frame into your application's input queue. If this function
	// takes too long, you'll start losing frames.
	void frame_callback(uvc_frame_t *frame) {
		Godot::print("uvc_camera_feed_callback triggered");
		/*
		uvc_frame_t *bgr;
		uvc_error_t ret;
		Ref<Image> p_rgb_img;
		PoolByteArray img_data;
		img_data.resize(3 * frame->width * frame->height);
		PoolByteArray::Write w = img_data.write();
		// We'll convert the image from YUV/JPEG to BGR, so allocate space
		bgr = uvc_allocate_frame(frame->width * frame->height * 3);
		if (!bgr) {
			LOGI("unable to allocate bgr frame!");
			return;
		}

		// Do the BGR conversion
		ret = uvc_any2bgr(frame, bgr);
		if (ret) {
			uvc_perror(ret, "uvc_any2bgr");
			uvc_free_frame(bgr);
			return;
		}
		memcpy(w.ptr(), bgr, 3 * frame->width * frame->height);

		p_rgb_img.instance();
		p_rgb_img->create(frame->width, frame->height, 0, Image::FORMAT_RGB8, img_data);
		set_RGB_img(p_rgb_img);

		uvc_free_frame(bgr);
		*/
	}

	int fd;
	uvc_device_handle* uvc_devh;
	struct uvc_stream_ctrl* stream_ctrl;
};

extern "C" void uvc_callback(struct uvc_frame* frame, void* userptr) {
	Godot::print("uvc_callback()");
	reinterpret_cast<CameraFeedUvc*>(userptr)->frame_callback(frame);
}

// Map file descriptors to associated feeds.
static std::map<int, Ref<CameraFeedUvc>> feeds;

static Ref<CameraFeedUvc> destroy(int fd) {
	auto p_feed = feeds.find(fd);
	if (p_feed == feeds.end())
		return {};
	Ref<CameraFeedUvc> feed{p_feed->second};
	feeds.erase(p_feed);

	// Make sure we stop recording if we are.
	feed->set_active(false);
	free(feed->stream_ctrl);
	uvc_close(feed->uvc_devh);
	feed->fd = -1;
	return feed;
}

static CameraFeedUvc *create(int fd, String name) {
	Godot::print(String{"create() fd="} + String::num_int64(fd));

	if (feeds.find(fd) != feeds.end())
		destroy(fd); // should have been destroyed already, do it now

	libusb_device_handle* usb_devh;
	int status = libusb_wrap_sys_device(usb_ctx, (intptr_t)fd, &usb_devh);
	if (status != 0) {
		Godot::print(String{"libusb_wrap_sys_device(): "} + String{libusb_error_name(status)});
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
		Godot::print(String{"uvc_open_with_usb_devh: "} + String{libuvc_error_name(status)});
		return {};
	}

	uvc_print_diag(uvc_devh, NULL);

	/* Try to negotiate a 160x120 14 fps YUYV stream profile */
	struct uvc_stream_ctrl* stream_ctrl =
		(struct uvc_stream_ctrl*)malloc(sizeof(struct uvc_stream_ctrl));
	status = uvc_get_stream_ctrl_format_size(
		uvc_devh, stream_ctrl, /* result stored in ctrl */
		UVC_FRAME_FORMAT_ANY, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
		640, 480, 15); /* width, height, fps */
	if (status != 0) {
		uvc_close(uvc_devh);
		Godot::print(String{"uvc_get_stream_ctrl_format_size: "} + String{libuvc_error_name(status)});
		return {};
	}
	uvc_print_stream_ctrl(stream_ctrl, NULL);

	auto *feed = CameraFeedUvc::_new();
	//feed->set_name(name);
	feed->fd = fd;
	feed->uvc_devh = uvc_devh;
	feed->stream_ctrl = stream_ctrl;

	//feeds.insert({fd, feed});
	return feed;
}

class UvcServer : public Reference {
	GODOT_CLASS(UvcServer, Reference);
public:
	UvcServer() { }

	void _init() {
		Godot::print("UvcServer::_init()");

		if (!usb_ctx && libusb_init(&usb_ctx))
			return;
		if (!uvc_ctx && uvc_init(&uvc_ctx, usb_ctx))
			return;

		auto *p = Engine::get_singleton();
		auto *cs = p->get_singleton("Uvc");
		if (!cs) {
			Godot::print("couldn’t load Uvc plugin");
			return;
		}

		if (cs->connect("connected", this, "connected") != Error::OK ||
		    cs->connect("disconnected", this, "disconnected") != Error::OK) {
			Godot::print("could not connect signals");
		}
	}

	Variant connected(Variant fd) {
		Godot::print(String{"UvcServer::connected("} + String::num_int64(fd) + String{")"});

		auto *p = Engine::get_singleton();
		auto *cs = Object::cast_to<CameraServer>(p->get_singleton("CameraServer"));
		if (cs) {
			CameraFeed* feed = create(int(fd), "XYZZY");
			auto rfeed = Ref<CameraFeed>(feed);
			feeds.insert({fd, rfeed});
			cs->add_feed(rfeed);
			return Variant{true};
		} else {
			Godot::print("couldn’t load CameraServer");
			return Variant{false};
		}
	}

	Variant disconnected(Variant fd) {
		return Variant{false};
	}

	static void _register_methods() {
		register_method("connected", &UvcServer::connected);
		register_method("disconnected", &UvcServer::disconnected);
	}
};

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
