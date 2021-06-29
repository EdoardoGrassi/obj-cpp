#find_package(GTest)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.10.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(gtest_build_gmock OFF)
FetchContent_MakeAvailable(googletest)


include(GoogleTest)

add_executable(obj_lexer_test "lexer_tests.cpp")
target_link_libraries(obj_lexer_test PRIVATE ${PROJECT_NAME} gtest_main)
gtest_discover_tests(obj_lexer_test)

add_executable(obj_parser_test "parser_tests.cpp")
target_link_libraries(obj_parser_test PRIVATE ${PROJECT_NAME} gtest_main)
gtest_discover_tests(obj_parser_test)