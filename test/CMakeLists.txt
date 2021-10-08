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
    "obj_parser_tests.cpp"
    "reader_tests.cpp"
    "mtl_parser_tests.cpp"
    "fuzzy_tests.cpp"
)
target_link_libraries(obj-cpp-tests PRIVATE Obj-cpp::obj-cpp gtest_main)

gtest_discover_tests(obj-cpp-tests
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/assets"
)