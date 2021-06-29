cmake_minimum_required (VERSION 3.8)
project ("obj-cpp")

option(OBJCPP_BUILD_TESTS "Build unit tests" ON)

add_library(${PROJECT_NAME} STATIC
    "src/lexer.cpp"
    "src/parser.cpp"
    "src/object.cpp"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_executable(obj-viewer "src/obj_viewer.cpp")
target_link_libraries(obj-viewer PRIVATE ${PROJECT_NAME})

if (OBJCPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory("test")
endif()