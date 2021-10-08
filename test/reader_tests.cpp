#include "obj-cpp/reader.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

using namespace obj;

GTEST_TEST(Reader, AssetFiles)
{
    std::ifstream file{ "cube.obj" };
    ASSERT_TRUE(file);
    std::stringstream source{};
    source << file.rdbuf();

    const auto dom = obj::parse_as_obj(source.str());

    EXPECT_EQ(std::size(dom.objects), 1);
    EXPECT_EQ(dom.objects[0].name, "Cube");

    // TODO: enable 'ArrayInitializerAlignmentStyle' option
    //  when clang-format 13 is supported by VS.

    const std::vector<Vertex> v = {
        // clang-format off
        {  1,  1, -1, 1 },
        {  1, -1, -1, 1 },
        {  1,  1,  1, 1 },
        {  1, -1,  1, 1 },
        { -1,  1, -1, 1 },
        { -1, -1, -1, 1 },
        { -1,  1,  1, 1 },
        { -1, -1,  1, 1 },
        // clang-format on
    };
    EXPECT_EQ(std::size(dom.data.v), std::size(v));
    EXPECT_EQ(dom.data.v, v);

    const std::vector<Normal> vn = {
        // clang-format off
        {  0.0,  1.0,  0.0 },
        {  0.0,  0.0,  1.0 },
        { -1.0,  0.0,  0.0 },
        {  0.0, -1.0,  0.0 },
        {  1.0,  0.0,  0.0 },
        {  0.0,  0.0, -1.0 },
        // clang-format on
    };
    EXPECT_EQ(std::size(dom.data.vn), 6);
    EXPECT_EQ(dom.data.vn, vn);

    const std::vector<Texcoord> vt = {
        { 0.875000, 0.500000, 0 },
        { 0.625000, 0.750000, 0 },
        { 0.625000, 0.500000, 0 },
        { 0.375000, 1.000000, 0 },
        { 0.375000, 0.750000, 0 },
        { 0.625000, 0.000000, 0 },
        { 0.375000, 0.250000, 0 },
        { 0.375000, 0.000000, 0 },
        { 0.375000, 0.500000, 0 },
        { 0.125000, 0.750000, 0 },
        { 0.125000, 0.500000, 0 },
        { 0.625000, 0.250000, 0 },
        { 0.875000, 0.750000, 0 },
        { 0.625000, 1.000000, 0 },
    };
    EXPECT_EQ(std::size(dom.data.vt), std::size(vt));
    EXPECT_EQ(dom.data.vt, vt);

    const std::vector<Face> faces = {
        // clang-format off
        { 5,  1, 1,     3,  2, 1,    1,  3, 1 },
        { 3,  2, 2,     8,  4, 2,    4,  5, 2 },
        { 7,  6, 3,     6,  7, 3,    8,  8, 3 },
        { 2,  9, 4,     8, 10, 4,    6, 11, 4 },
        { 1,  3, 5,     4,  5, 5,    2,  9, 5 },
        { 5, 12, 6,     2,  9, 6,    6,  7, 6 },
        { 5,  1, 1,     7, 13, 1,    3,  2, 1 },
        { 3,  2, 2,     7, 14, 2,    8,  4, 2 },
        { 7,  6, 3,     5, 12, 3,    6,  7, 3 },
        { 2,  9, 4,     4,  5, 4,    8, 10, 4 },
        { 1,  3, 5,     3,  2, 5,    4,  5, 5 },
        { 5, 12, 6,     1,  3, 6,    2,  9, 6 },
        // clang-format on
    };
    EXPECT_EQ(std::size(dom.data.faces), std::size(faces));
    EXPECT_EQ(dom.data.faces, faces);
}