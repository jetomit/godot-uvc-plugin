#ifndef UVC_SERVER_H
#define UVC_SERVER_H

#include <Godot.hpp>
#include <Reference.hpp>

class UvcServer : public godot::Reference {
	GODOT_CLASS(UvcServer, Reference);
public:
	void _init();
	void connected(godot::Variant fd);
	void disconnected(godot::Variant fd);

	static void _register_methods() {
		register_method("connected", &UvcServer::connected);
		register_method("disconnected", &UvcServer::disconnected);
	}
};

#endif
