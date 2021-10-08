cmake_minimum_required(VERSION 3.8)
project("obj-cpp" LANGUAGES CXX)

option(OBJ_CPP_BUILD_TESTS "Build unit tests" ON)
option(OBJ_CPP_PBR_EXTENSION "Enable support for PBR extension in material files" ON)

add_library(obj-cpp STATIC
    "src/lexer.cpp"
    "src/obj_parser.cpp"   
    "src/mtl_parser.cpp"
)
add_library(Obj-cpp::obj-cpp ALIAS obj-cpp)

target_compile_features(obj-cpp PUBLIC cxx_std_20)

if (MSVC)
    #target_compile_options(obj-cpp PRIVATE "/W4")
endif()

if (OBJ_CPP_PBR_EXTENSION)
    target_compile_definitions(obj-cpp PUBLIC OBJCPP_PBR_EXT)
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
    NAMESPACE Obj-cpp::
)

if (OBJ_CPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory("test")
endif()