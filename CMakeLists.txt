cmake_minimum_required (VERSION 3.8)
project ("obj-cpp")

option(OBJCPP_BUILD_TESTS "Build unit tests" ON)

add_library(obj-cpp STATIC
    "src/lexer.cpp"
    "src/parser.cpp"
    "src/core.cpp"
)

target_compile_features(obj-cpp PUBLIC cxx_std_20)

if (MSVC)
    #target_compile_options(obj-cpp PRIVATE "/W4")
endif()

target_include_directories(
    obj-cpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_executable(obj-viewer "src/viewer.cpp")
target_link_libraries(obj-viewer PRIVATE obj-cpp)

install(
    TARGETS obj-cpp
    EXPORT ${PROJECT_NAME}-targets
)
export(
    EXPORT ${PROJECT_NAME}-targets
    NAMESPACE Obj
)

if (OBJCPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory("test")
endif()