#include "uvc-server.h"

#include <map>

#include <CameraFeed.hpp>
#include <CameraServer.hpp>
#include <Defs.hpp>
#include <Engine.hpp>

#include "uvc-feed.h"

using namespace godot;

// Map file descriptors to associated feeds.
static std::map<int, Ref<CameraFeedUvc>> feeds;

void UvcServer::_init() {
	Godot::print("UvcServer::_init()");

	auto *uvc = Engine::get_singleton()->get_singleton("Uvc");
	ERR_FAIL_COND(!uvc);
	uvc->connect("connected", this, "connected");
	uvc->connect("disconnected", this, "disconnected");
}

void UvcServer::connected(int fd, String name) {
	Godot::print(String{"UvcServer::connected("} + String::num_int64(fd) + String{")"});

	disconnected(fd); // this is probably not necessary

	auto *camera_server = Object::cast_to<CameraServer>(
		Engine::get_singleton()->get_singleton("CameraServer"));
	ERR_FAIL_COND(!camera_server);

	auto feed = Ref<CameraFeed>(CameraFeedUvc::create(fd, name));
	feeds.insert({fd, feed});
	camera_server->add_feed(feed);
}

void UvcServer::disconnected(int fd) {
	Godot::print(String{"UvcServer::disconnected("} + String::num_int64(fd) + String{")"});

	auto *camera_server = Object::cast_to<CameraServer>(
		Engine::get_singleton()->get_singleton("CameraServer"));
	ERR_FAIL_COND(!camera_server);

	auto p_feed = feeds.find(fd);
	if (p_feed != feeds.end()) {
		camera_server->remove_feed(p_feed->second);
		CameraFeedUvc::destroy(p_feed->second);
		feeds.erase(p_feed);
	}
}
