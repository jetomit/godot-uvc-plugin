cmake_minimum_required(VERSION 3.6)

project(godot_uvc)

file(GLOB_RECURSE SOURCES src/main/cpp/*.c**)
file(GLOB_RECURSE HEADERS src/main/cpp/*.h**)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
