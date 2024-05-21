cmake_minimum_required(VERSION 3.28)
project(learn_opengl)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp GLAD/src/glad.c)
set(HEADER_FILES shader.h)

include_directories(include)
include_directories(GLAD/include)

add_executable(learn_opengl main.cpp)

target_link_libraries(learn_opengl -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi)


