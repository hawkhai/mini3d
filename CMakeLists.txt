cmake_minimum_required(VERSION 3.15)

project(Mini3D)

add_executable(${PROJECT_NAME}
    mini3d.h
    math.cpp
    transform.cpp
    device.cpp
    screen.cpp
    mini3d.cpp
)

set_target_properties(Mini3D
    PROPERTIES
    PLATFORM_TOOLSET
    "v141") 

# print message
message(${PROJECT_SOURCE_DIR})

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif
