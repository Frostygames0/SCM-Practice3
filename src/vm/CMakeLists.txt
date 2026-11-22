add_library(interpreter interpreter.hpp interpreter.cpp)
target_include_directories(interpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(interpreter PUBLIC codec PRIVATE SCM3_shared)

add_executable(vm main.cpp)
target_link_libraries(vm PRIVATE SCM3_shared interpreter CLI11::CLI11)
