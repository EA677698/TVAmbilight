cmake_minimum_required(VERSION 3.24)
project(TVAmbilight)

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/rpi_ws281x)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

find_package(rpi_ws281x)

add_executable(TVAmbilight src/main.cpp)

target_link_libraries(TVAmbilight ${OpenCV_LIBS})