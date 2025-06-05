cmake_minimum_required(VERSION 4.0.2)
project(tic-tac-toe)

set(CMAKE_CXX_STANDARD 23)

include_directories(${CMAKE_SOURCE_DIR}/headers)
include_directories(${CMAKE_SOURCE_DIR}/extern/imgui)
include_directories(${CMAKE_SOURCE_DIR}/extern/glfw/include)

add_subdirectory(extern/glfw)

file(GLOB IMGUI_SOURCES
    "${CMAKE_SOURCE_DIR}/extern/imgui/*.cpp"
    "${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_opengl3.cpp"
)
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/sources/*.cpp")

add_executable(tic-tac-toe ${SOURCES} ${IMGUI_SOURCES})

target_link_libraries(tic-tac-toe glfw OpenGL32)
