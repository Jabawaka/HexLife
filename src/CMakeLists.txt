cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -Wall -Wextra")

project(HexLife VERSION 1.0)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../bin/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../lib/)

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
include_directories(
    /Library/Frameworks/SDL2.framework/Versions/A/Headers
    /Library/Frameworks/SDL2_image.framework/Versions/A/Headers
    /Library/Frameworks/SDL2_ttf.framework/Versions/A/Headers
)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../lib/)


add_executable(HexLife main.cpp)
target_link_libraries(HexLife PRIVATE
    /Library/Frameworks/SDL2.framework/Versions/A/SDL2
    /Library/Frameworks/SDL2_image.framework/Versions/A/SDL2_image
    /Library/Frameworks/SDL2_ttf.framework/Versions/A/SDL2_ttf
)