#include "uvc-feed.h"

#include <cstdlib>

#include <libusb.h>
#include <libuvc/libuvc_internal.h>

#include <Defs.hpp>

using namespace godot;

static uvc_context* uvc_ctx{nullptr};

static const char* libuvc_error_name (int e) {
	return uvc_strerror((uvc_error_t)e);
}

void CameraFeedUvc::_init() {
	Godot::print("CameraFeedUvc::_init()");
}

// XXX without copying this from CameraFeed, base class
// (de)activate_feed methods get called even for CameraFeedUvc
void CameraFeedUvc::set_active(bool is_active) {
	Godot::print("CameraFeedUvc::set_active()");
	if (is_active != active) {
		if (is_active) {
			if (activate_feed()) {
				Godot::print("Activate ");
				active = true;
			}
		} else {
			deactivate_feed();
			Godot::print("Deactivate ");
			active = false;
		}
	}
}

bool CameraFeedUvc::activate_feed() {
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

void CameraFeedUvc::deactivate_feed() {
	Godot::print("CameraFeedUvc::deactivate_feed()");
};

CameraFeedUvc* CameraFeedUvc::create(int fd, String name) {
	Godot::print(String{"CameraFeedUvc::create() fd="} + String::num_int64(fd));

	if (!uvc_ctx && uvc_init(&uvc_ctx, nullptr))
		return nullptr;

	// Set up device handle.
	libusb_device_handle* usb_devh;
	int status = libusb_wrap_sys_device(uvc_ctx->usb_ctx, (intptr_t)fd, &usb_devh);
	if (status != 0) {
		Godot::print(String{"libusb_wrap_sys_device(): "} + String{libusb_error_name(status)});
		return nullptr;
	}

	uvc_device* uvc_dev;
	uvc_dev = (uvc_device*)malloc(sizeof(uvc_device)); // XXX THIS LEAKS!!??
	uvc_dev->ctx = uvc_ctx;
	uvc_dev->ref = 0;
	uvc_dev->usb_dev = libusb_get_device(usb_devh);

	uvc_device_handle* uvc_devh;
	status = uvc_open_with_usb_devh(uvc_dev, &uvc_devh, usb_devh);
	if (status != 0) {
		Godot::print(String{"uvc_open_with_usb_devh: "} + String{libuvc_error_name(status)});
		return nullptr;
	}
	uvc_print_diag(uvc_devh, NULL);

	// Select a stream profile.
	struct uvc_stream_ctrl* stream_ctrl =
		(struct uvc_stream_ctrl*)malloc(sizeof(struct uvc_stream_ctrl));
	status = uvc_get_stream_ctrl_format_size(
			uvc_devh, stream_ctrl, /* result stored in ctrl */
			UVC_FRAME_FORMAT_ANY,
			640, 480, 15); /* width, height, fps */
	if (status != 0) {
		uvc_close(uvc_devh);
		Godot::print(String{"uvc_get_stream_ctrl_format_size: "} + String{libuvc_error_name(status)});
		return nullptr;
	}
	uvc_print_stream_ctrl(stream_ctrl, NULL);

	auto *feed = CameraFeedUvc::_new();
	//feed->set_name(name);
	feed->_set_name(name);
	feed->fd = fd;
	feed->uvc_devh = uvc_devh;
	feed->stream_ctrl = stream_ctrl;
	return feed;
}

void CameraFeedUvc::destroy(Ref<CameraFeedUvc> feed) {
	Godot::print(String{"CameraFeedUvc::destroy() fd="} + String::num_int64(feed->fd));
        // Make sure we stop recording if we are.
        feed->set_active(false);
        std::free(feed->stream_ctrl);
        uvc_close(feed->uvc_devh);
        feed->fd = -1;
}

// This callback function runs once per frame. Use it to perform any
// quick processing you need, or have it put the frame into your
// application's input queue. If this function takes too long, you'll
// start losing frames.
void CameraFeedUvc::uvc_callback(struct uvc_frame* frame, void* userptr) {
	Godot::print("CameraFeedUvc::uvc_callback()");
	auto feed = reinterpret_cast<CameraFeedUvc*>(userptr);

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
