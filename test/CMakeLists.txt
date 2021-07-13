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

add_executable(obj-cpp-tests
    "lexer_tests.cpp"
    "parser_tests.cpp"
)
target_link_libraries(obj-cpp-tests PRIVATE obj-cpp gtest_main)
gtest_discover_tests(obj-cpp-tests)