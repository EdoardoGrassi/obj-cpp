#include "obj-cpp/core.hpp"
#include "obj-cpp/lexer.hpp"
#include "obj-cpp/parser.hpp"

#include "gtest/gtest.h"

using namespace obj;

GTEST_TEST(Parser, Comments)
{
    char source[] = "# aaaaaaaaaaaaaaaaaaa\n"
                    "# bbbbbbbbbbbbbbbbbbb\n"
                    "# ccccccccccccccccccc\n"
                    "# ddddddddddddddddddd\n\0";

    const auto r = obj::parse(source, {});
    ASSERT_EQ(std::size(r.data.v), 0);
    ASSERT_EQ(std::size(r.data.vn), 0);
    ASSERT_EQ(std::size(r.data.vt), 0);
    ASSERT_EQ(std::size(r.data.faces), 0);
}

GTEST_TEST(Parser, Vertices)
{
    char source[] = "# this is a comment\n"
                    "v 1.0 2.0 3.0\n"
                    "v 1.0 2.0 3.0\n"
                    "v 1.0 2.0 3.0\n"
                    "v 1.0 2.0 3.0\n\0";

    const auto tokens = obj::lex(source);
    for (const auto& t : tokens)
        std::cout << t << '\n';

    const auto r = obj::parse(source, {});
    ASSERT_EQ(std::size(r.data.v), 4);
    for (const auto& v : r.data.v)
        ASSERT_EQ(v, obj::Vertex(1.f, 2.f, 3.f, 1.f));
}

GTEST_TEST(Parser, Normals)
{
    char source[] = "# this is a comment\n"
                    "vn 1.0 2.0 3.0\n"
                    "vn 1.0 2.0 3.0\n"
                    "vn 1.0 2.0 3.0\n"
                    "vn 1.0 2.0 3.0\n";

    const auto dom = obj::parse(source, {});
    ASSERT_EQ(std::size(dom.data.vn), 4);
    for (const auto& vn : dom.data.vn)
        ASSERT_EQ(vn, obj::Normal(1.f, 2.f, 3.f));
}