file(GLOB sources "*.cpp" "*.h")

add_library(net ${sources})
target_link_libraries(net PUBLIC tools)