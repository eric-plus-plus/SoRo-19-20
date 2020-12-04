cmake_minimum_required(VERSION 2.8)

project(autonomous)

include_directories("${PROJECT_SOURCE_DIR}")
add_subdirectory("${PROJECT_SOURCE_DIR}/gps")

set(HEADER_FILES libs/DriveMode.cpp libs/DriveMode.h libs/Location.cpp libs/Location.h libs/ARTracker.cpp libs/ARTracker.h libs/UDPOut.cpp libs/UDPOut.cpp)

find_package( OpenCV REQUIRED )

add_executable(autonomous main.cpp ${HEADER_FILES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

target_link_libraries( autonomous gps )
target_link_libraries( autonomous  ${OpenCV_LIBS})
target_link_libraries( autonomous  aruco )
#target_link_libsraries( autonomous ${CMAKE_THREAD_libs_INIT} ) 

