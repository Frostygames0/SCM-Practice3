add_executable(tests
    codec_test.cpp
    assembler_test.cpp
    interpreter_test.cpp
)
target_link_libraries(tests PRIVATE SCM3_shared assembler interpreter GTest::gtest_main)
target_compile_definitions(tests PRIVATE EXAMPLES_DIR="${PROJECT_SOURCE_DIR}/examples")

include(GoogleTest)
gtest_discover_tests(tests)
