cmake_minimum_required(VERSION 3.6)

project(godot_uvc)

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

file(GLOB_RECURSE SOURCES src/main/cpp/*.cpp**)
file(GLOB_RECURSE HEADERS src/main/cpp/*.h**)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} usb uvc)
