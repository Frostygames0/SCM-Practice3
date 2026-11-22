add_library(assembler assembler.hpp assembler.cpp)
target_include_directories(assembler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(assembler PUBLIC codec PRIVATE SCM3_shared)

add_executable(asm main.cpp)
target_link_libraries(asm PRIVATE SCM3_shared assembler CLI11::CLI11)
