cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)

project(godot_uvc)

# godot-cpp
file(GLOB_RECURSE GODOT_CPP_SOURCES libs/godot-cpp/src/*.cpp**)
file(GLOB_RECURSE GODOT_CPP_HEADERS libs/godot-cpp/include/*.hpp**)

add_library(godot-cpp STATIC ${GODOT_CPP_SOURCES} ${GODOT_CPP_HEADERS})

target_include_directories(godot-cpp PUBLIC
        libs/godot-cpp/include
        libs/godot-cpp/include/core
        libs/godot-cpp/include/gen
)

# Put godot headers as SYSTEM PUBLIC to exclude warnings from irrelevant headers
target_include_directories(godot-cpp SYSTEM PUBLIC libs/godot-cpp/godot_headers)

set(GODOT_LINKER_FLAGS "-Wl")
set(GODOT_COMPILE_FLAGS "-fPIC -g -Wwrite-strings")

set_property(TARGET godot-cpp APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
set_property(TARGET godot-cpp APPEND_STRING PROPERTY LINK_FLAGS ${GODOT_LINKER_FLAGS})

# libusb
add_library(usb SHARED
	libs/libusb/libusb/core.c
	libs/libusb/libusb/descriptor.c
	libs/libusb/libusb/hotplug.c
	libs/libusb/libusb/io.c
	libs/libusb/libusb/sync.c
	libs/libusb/libusb/strerror.c
	libs/libusb/libusb/os/linux_usbfs.c
	libs/libusb/libusb/os/poll_posix.c
	libs/libusb/libusb/os/threads_posix.c
	libs/libusb/libusb/os/linux_netlink.c
)
target_link_libraries(usb log)

# libuvc
add_library(uvc SHARED
	libs/libuvc/src/ctrl.c
	libs/libuvc/src/ctrl-gen.c
	libs/libuvc/src/device.c
	libs/libuvc/src/diag.c
	libs/libuvc/src/frame.c
	libs/libuvc/src/init.c
	libs/libuvc/src/stream.c
	libs/libuvc/src/misc.c
)
target_link_libraries(uvc usb)

include_directories(libs/libusb/libusb)
include_directories(libs/libuvc/include)

# plugin: gdscript
add_library(uvc-server SHARED src/gdscript/uvc-server.cpp)
target_include_directories(uvc-server PUBLIC
        libs/godot-cpp/include
        libs/godot-cpp/include/core
        libs/godot-cpp/include/gen
)
set_property(TARGET uvc-server APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
target_include_directories(uvc-server SYSTEM PUBLIC libs/godot-cpp/godot_headers)
target_link_libraries(uvc-server godot-cpp usb uvc)

# plugin: JNI part
file(GLOB_RECURSE SOURCES src/main/cpp/*.cpp**)
file(GLOB_RECURSE HEADERS src/main/cpp/*.h**)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} godot-cpp usb uvc)

set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS ${GODOT_LINKER_FLAGS})
