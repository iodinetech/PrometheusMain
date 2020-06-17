cmake_minimum_required(VERSION 3.4.0)
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}")
set(CXX_FLAGS
    -O2
    --std=c++17
    )

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "protobuf found.")
endif()

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(kademlia)
add_subdirectory(net)
add_subdirectory(tools)
add_subdirectory(console)
add_executable(kadfiletransporter main.cpp)

target_link_libraries(
    kadfiletransporter
    PRIVATE kademlia net tools boost_system)
