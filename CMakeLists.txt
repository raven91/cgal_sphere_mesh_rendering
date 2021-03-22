cmake_minimum_required(VERSION 3.17)
project(cgal_sphere_mesh_rendering)

set(CMAKE_CXX_STANDARD 17)

add_executable(cgal_sphere_mesh_rendering main.cpp
        Renderer.cpp Renderer.hpp
        ScreenshotHandler.cpp ScreenshotHandler.hpp)

include_directories(/usr/local/include/ /usr/local/include/freetype2/)
link_directories(/usr/local/lib/)

file(GLOB LIBRARIES "/usr/local/lib/libGLEW.1.13.0.dylib"
        "/usr/local/lib/libglfw.3.1.dylib"
        "/usr/local/lib/libpng16.16.dylib"
        "/usr/local/lib/libfreetype.a"
        "/usr/local/opt/bzip2/lib/libbz2.a"
        "/usr/local/opt/zlib/lib/libz.a")
message("LIBRARIES = ${LIBRARIES}")
target_link_libraries(cgal_sphere_mesh_rendering ${LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenGL")