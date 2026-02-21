cmake_minimum_required(VERSION 3.28)
project(learn_opengl)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(learn_opengl ${SOURCES})

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

# Copy shaders and assets to build directory on every build
add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:learn_opengl>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:learn_opengl>/shaders
)
add_dependencies(learn_opengl copy_shaders)

add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:learn_opengl>/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:learn_opengl>/assets
)
add_dependencies(learn_opengl copy_assets)
