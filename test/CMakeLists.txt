cmake_minimum_required(VERSION 2.6)
project(testednio)

if(NOT CMAKE_BUILD_TYPE )
	set(CMAKE_BUILD_TYPE Release)
endif()	
set(CMAKE_CXX_FLAGS "-std=c++0x -pthread " )
file(GLOB ALLSRCS "*.cpp" )

include_directories("${PROJECT_SOURCE_DIR}")
link_directories("${PROJECT_SOURCE_DIR}/../Debug")
add_executable(testednio ${ALLSRCS})
target_link_libraries(testednio "pthread" "ednio" "gtest")
