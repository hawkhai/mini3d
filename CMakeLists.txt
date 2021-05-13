cmake_minimum_required(VERSION 3.15)

project(Mini3D)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

add_definitions(-DUNICODE -D_UNICODE)

add_executable(${PROJECT_NAME}
    mini3d.h
    math.h
    math.cpp
    transform.cpp
    device.cpp
    window.h
    window.cpp
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
