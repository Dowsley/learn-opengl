cmake_minimum_required(VERSION 3.28)
project(learn_opengl)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp GLAD/src/glad.c)
set(HEADER_FILES shader.h)

include_directories(include)
include_directories(GLAD/include)

add_executable(learn_opengl main.cpp
        GLAD/src/glad.c
        shader.cpp
        lib/stb_image.cpp
)

if(WIN32)
    # Use the MinGW pre-built GLFW library
    set(GLFW3_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib/libglfw3.a")

    if(EXISTS ${GLFW3_LIB_PATH})
        add_library(glfw STATIC IMPORTED)
        set_target_properties(glfw PROPERTIES
                IMPORTED_LOCATION ${GLFW3_LIB_PATH}
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/GLFW")
        target_link_libraries(learn_opengl glfw)
    else()
        message(FATAL_ERROR "libglfw3.a not found at ${GLFW3_LIB_PATH}")
    endif()
elseif(UNIX)
    target_link_libraries(learn_opengl -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi)
endif()

add_custom_command(
        TARGET learn_opengl PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:learn_opengl>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:learn_opengl>/shaders
)

add_custom_command(
        TARGET learn_opengl PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:learn_opengl>/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:learn_opengl>/assets
)
