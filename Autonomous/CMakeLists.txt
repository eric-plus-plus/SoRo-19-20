cmake_minimum_required(VERSION 2.8)

project(autonomous)
include_directories("${PROJECT_SOURCE_DIR}")
add_subdirectory("${PROJECT_SOURCE_DIR}/gps")

set(HEADER_FILES Location.cpp Location.h ARTracker.cpp ARTracker.h)

add_executable(autonomous LocationTester.cpp ${HEADER_FILES})

find_package( OpenCV REQUIRED )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

target_link_libraries( autonomous gps )
target_link_libraries( autonomous  ${OpenCV_LIBS})
target_link_libraries( autonomous  aruco )
#target_link_libraries( autonomous ${CMAKE_THREAD_LIBS_INIT} ) 

