cmake_minimum_required(VERSION 4.0.0)
project(BiotSavartSim VERSION 2.0.0)

add_executable(BiotSavartSim src/config.h src/main.cpp src/glad.c)

target_include_directories(BiotSavartSim PRIVATE dependencies)

target_link_libraries(BiotSavartSim PRIVATE glfw3 opengl32 gdi32)