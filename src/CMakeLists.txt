cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)
if (APPLE OR UNIX)
    set(CMAKE_C_FLAGS "-g -Wall -Wextra")
endif()

if (APPLE)
    list(APPEND CMAKE_INSTALL_RPATH /Library/Frameworks/)
    list(APPEND CMAKE_BUILD_RPATH /Library/Frameworks/)
endif (APPLE)

project(HexLife VERSION 1.0)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../bin/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../lib/)

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR})

set(SDL2_DIR "$ENV{DEVLIB_ROOT}/SDL/SDL2-2.30.2/cmake")
set(SDL2_image_DIR "$ENV{DEVLIB_ROOT}/SDL/SDL2_image-2.8.2/cmake")
set(SDL2_ttf_DIR "$ENV{DEVLIB_ROOT}/SDL/SDL2_ttf-2.22.0/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(HexLife main.c)
target_link_libraries(HexLife PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
)