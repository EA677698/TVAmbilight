cmake_minimum_required(VERSION 3.18)
project(TVAmbilight)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

include_directories(/usr/local/include)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

link_directories(/usr/local/lib)

add_executable(TVAmbilight src/main.cpp)

target_link_libraries(TVAmbilight ${OpenCV_LIBS} ws2811)