cmake_minimum_required(VERSION 3.28)
project(learn_opengl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find dependencies (provided by vcpkg)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")

add_executable(learn_opengl
        src/main.cpp
        src/application.cpp
        src/render/shader.cpp
        src/render/stb_impl.cpp
)

target_include_directories(learn_opengl PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${STB_INCLUDE_DIRS}
)

target_link_libraries(learn_opengl PRIVATE
        glfw
        glad::glad
        glm::glm
)

if(APPLE)
    target_link_libraries(learn_opengl PRIVATE
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
    )
endif()

# Copy shaders and assets to build directory
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
