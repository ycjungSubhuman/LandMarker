cmake_minimum_required(VERSION 3.11)
project(LandMarker)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 17)

option(LIBIGL_WITH_OPENGL            "Use OpenGL"         ON)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"           ON)

find_package(LIBIGL REQUIRED QUIET)

add_library(imgui 
    3rdparty/imgui/imgui_demo.cpp
    3rdparty/imgui/imgui_draw.cpp
    3rdparty/imgui/imgui_tables.cpp
    3rdparty/imgui/imgui_widgets.cpp
    3rdparty/imgui/imgui.cpp
    3rdparty/imgui/backends/imgui_impl_glfw.cpp
    3rdparty/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC 
    3rdparty/
    3rdparty/imgui/
    3rdparty/imgui/backends/)
add_library(tinyobjloader
    3rdparty/tinyobjloader/tiny_obj_loader.cc)
target_include_directories(tinyobjloader PUBLIC 
    3rdparty/tinyobjloader)

add_executable(LandMarker src/main.cpp)
target_include_directories(LandMarker PRIVATE include/)
target_link_libraries(LandMarker PUBLIC igl::core igl::opengl_glfw imgui tinyobjloader)
