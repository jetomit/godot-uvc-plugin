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

	// XXX without copying this from CameraFeed, base class
	// (de)activate_feed methods get called even for CameraFeedUvc
	void set_active(bool p_is_active);

	bool activate_feed();
	void deactivate_feed();

	static CameraFeedUvc* create(int fd, godot::String name);
	static void destroy(godot::Ref<CameraFeedUvc> feed);

	static void _register_methods() {
		register_method("activate_feed", &CameraFeedUvc::activate_feed);
		register_method("deactivate_feed", &CameraFeedUvc::deactivate_feed);
		register_method("set_active", &CameraFeedUvc::set_active);
	};

private:
	int fd;
	uvc_device_handle* uvc_devh;
	struct uvc_stream_ctrl* stream_ctrl;

	bool active;
	static void uvc_callback(struct uvc_frame* frame, void* userptr);
};


#endif
