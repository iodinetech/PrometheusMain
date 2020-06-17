file(GLOB sources "*.cpp" "*.h")

add_library(tools ${sources})
target_link_libraries(tools -lpthread)