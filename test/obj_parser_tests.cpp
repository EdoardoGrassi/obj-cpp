#include "obj-cpp/obj_parser.hpp"
#include "obj-cpp/parser.hpp"

#include "gtest/gtest.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace obj;

namespace obj
{
    //template <class Value>
    inline std::ostream& operator<<(std::ostream& os, const Vertex& v)
    {
        return os << '(' << v.x << ',' << v.y << ',' << v.z << ',' << v.w << ')';
    }

    //template <class Value>
    inline std::ostream& operator<<(std::ostream& os, const Normal& n)
    {
        return os << '(' << n.x << ',' << n.y << ',' << n.z << ')';
    }

    //template <class Value>
    inline std::ostream& operator<<(std::ostream& os, const Texcoord& n)
    {
        return os << '(' << n.u << ',' << n.v << ',' << n.w << ')';
    }

    inline std::ostream& operator<<(std::ostream& os, const Triplet& t)
    {
        return os << '(' << t.v << ',' << t.vt << ',' << t.vn << ')';
    }

    inline std::ostream& operator<<(std::ostream& os, const Face& f)
    {
        return os << '{' << f.triplets[0] << ',' << f.triplets[1] << ',' << f.triplets[2] << '}';
    }

} // namespace obj

GTEST_TEST(ObjParser, Comments)
{
    const std::string source = "# aa\n"
                               "#bbb\n"
                               "# c # c #c #c\n"
                               "#############\n";

    const auto dom = obj::parse_as_obj(source);
    ASSERT_EQ(std::size(dom.data.v), {});
    ASSERT_EQ(std::size(dom.data.vn), {});
    ASSERT_EQ(std::size(dom.data.vt), {});
    ASSERT_EQ(std::size(dom.data.faces), {});
}

GTEST_TEST(ObjParser, Vertices)
{
    const std::string source = "v 1.0 1.0 1.0     # (x, y, z)\n"
                               "v 2.0 2.0 2.0     # (x, y, z)\n"
                               "v 3.0 3.0 3.0 3.0 # (x, y, z, w)\n"
                               "v 4.0 4.0 4.0 4.0 # (x, y, z, w)\n";

    const auto dom = obj::parse_as_obj(source);

    const std::vector<obj::Vertex> v = {
        { 1.f, 1.f, 1.f, 1.f },
        { 2.f, 2.f, 2.f, 1.f },
        { 3.f, 3.f, 3.f, 3.f },
        { 4.f, 4.f, 4.f, 4.f },
    };
    ASSERT_EQ(dom.data.v, v);

    const std::string s = "v 1.0 1.0\n";
    EXPECT_THROW(auto _ = obj::parse_as_obj(s), ParserError);
}

GTEST_TEST(ObjParser, Normals)
{
    const std::string source = "vn 1.0 1.0 1.0\n"
                               "vn 2.0 2.0 2.0\n"
                               "vn 3.0 3.0 3.0\n"
                               "vn 4.0 4.0 4.0\n";
    const std::vector<Normal> vn = {
        { 1.f, 1.f, 1.f },
        { 2.f, 2.f, 2.f },
        { 3.f, 3.f, 3.f },
        { 4.f, 4.f, 4.f },
    };

    const auto dom = obj::parse_as_obj(source);
    ASSERT_EQ(dom.data.vn, vn);
}

GTEST_TEST(ObjParser, Texcoords)
{
    const std::string source = "vt 1.0         # single component\n"
                               "vt 2.0 2.0     # (u, v) \n"
                               "vt 3.0 3.0 3.0 # (u, v, w)\n";
    const std::vector<Texcoord> vt = {
        { 1.f, 0.f, 0.f },
        { 2.f, 2.f, 0.f },
        { 3.f, 3.f, 3.f },
    };

    const auto dom = obj::parse_as_obj(source);
    ASSERT_EQ(dom.data.vt, vt);
}

GTEST_TEST(ObjParser, Triplets)
{
    ASSERT_EQ(parse_triplet("1/1/1"), Triplet(1, 1, 1));
    ASSERT_EQ(parse_triplet("1/2/3"), Triplet(1, 2, 3));
    EXPECT_THROW(auto _ = parse_triplet("/11/22"), ParserError);
}

GTEST_TEST(ObjParser, Faces)
{
    const std::string source = "v 1.0 1.0 1.0\n"
                               "v 2.0 2.0 2.0\n"
                               "v 3.0 3.0 3.0\n"
                               "v 4.0 4.0 4.0\n"
                               "f 1//   2//   3//\n"
                               "f 1/1/  2/2/  3/3/\n"
                               "f 1/1/1 2/2/2 3/3/3\n";

    const auto dom = obj::parse_as_obj(source);

    const std::vector<Face> f = {
        { 1, 0, 0,   2, 0, 0,   3, 0, 0 },
        { 1, 1, 0,   2, 2, 0,   3, 3, 0 },
        { 1, 1, 1,   2, 2, 2,   3, 3, 3 },
    };
    ASSERT_EQ(dom.data.faces, f);
}