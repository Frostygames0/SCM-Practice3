add_library(codec codec.hpp codec.cpp)

target_link_libraries(codec PRIVATE SCM3_shared)
target_include_directories(codec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})