cmake_minimum_required(VERSION 2.8)

project(collectData)

include_directories("${PROJECT_SOURCE_DIR}")

find_package( OpenCV REQUIRED )

add_executable(collectData analyze.cpp)

target_link_libraries( collectData  ${OpenCV_LIBS})
