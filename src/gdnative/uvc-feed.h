#ifndef UVC_FEED_H
#define UVC_FEED_H

#include <libuvc/libuvc.h>

#include <CameraFeed.hpp>
#include <Godot.hpp>
#include <Reference.hpp>

class CameraFeedUvc : public godot::CameraFeed {
	GODOT_CLASS(CameraFeedUvc, CameraFeed);
public:
	void _init();

	void set_active(bool active);

	static CameraFeedUvc* create(int fd, godot::String name);
	static void destroy(godot::Ref<CameraFeedUvc> feed);

	static void _register_methods() {
		register_method("set_active", &CameraFeedUvc::set_active);
	};

private:
	int fd;
	uvc_device_handle* uvc_devh;
	struct uvc_stream_ctrl* stream_ctrl;
	static void uvc_callback(struct uvc_frame* frame, void* userptr);
};


#endif
