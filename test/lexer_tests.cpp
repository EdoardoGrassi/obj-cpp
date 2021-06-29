#include "lexer.hpp"

#include "gtest/gtest.h"

#include <algorithm>
#include <iostream>

using namespace obj;

void print(const std::span<const Token> tokens)
{
    for (const auto& t : tokens)
        std::cout << t << '\n';
}

GTEST_TEST(Lexer, SingleLineComment)
{
    char       text[] = "# this is a single line comment\0";
    const auto tokens = lex(text);
    ASSERT_TRUE(std::empty(tokens));
}

GTEST_TEST(Lexer, MultiLineComments)
{
    char text[] = "# this is\n"
                  "# a comment\n"
                  "# on multiple lines\0";
    const Token expected[] = {
        { Token::Type::endline, "\n" },
        { Token::Type::endline, "\n" }
    };

    const auto ts = lex(text);
    ASSERT_EQ(std::size(ts), std::size(expected));
    ASSERT_TRUE(std::equal(std::cbegin(ts), std::cend(ts), std::begin(expected)));
}

GTEST_TEST(Lexer, MultiComments)
{
    char text[] = "# this is a comment\n"
                  "   # this is a comment too\n"
                  "        # yet another #one\n\0";
    const Token expected[] = {
        { Token::Type::endline, "\n" },
        { Token::Type::endline, "\n" },
        { Token::Type::endline, "\n" }
    };

    const auto ts = lex(text);
    ASSERT_EQ(std::size(ts), std::size(expected));
    ASSERT_TRUE(std::equal(std::cbegin(ts), std::cend(ts), std::begin(expected)));
}

GTEST_TEST(Lexer, Alphanum)
{
    char text[] = "aa bb\n"
                  "1.2345 1.2345\0";
    const Token expected[] = {
        { Token::Type::alphanum, "aa" },
        { Token::Type::alphanum, "bb" },
        { Token::Type::endline, "\n" },
        { Token::Type::alphanum, "1.2345" },
        { Token::Type::alphanum, "1.2345" }
    };

    const auto ts = lex(text);
    ASSERT_EQ(std::size(ts), std::size(expected));
    ASSERT_TRUE(std::equal(std::cbegin(ts), std::cend(ts), std::begin(expected)));
}

GTEST_TEST(Lexer, MixedTokenTypes)
{
    char text[] = "aa bb\n"
                  "# comment line # still a comment\n"
                  "cc 123\n\0";
    const Token expected[] = {
        { Token::Type::alphanum, "aa" },
        { Token::Type::alphanum, "bb" },
        { Token::Type::endline, "\n" },
        { Token::Type::endline, "\n" },
        { Token::Type::alphanum, "cc" },
        { Token::Type::alphanum, "123" },
        { Token::Type::endline, "\n" }
    };

    const auto ts = lex(text);
    ASSERT_EQ(std::size(ts), std::size(expected));
    ASSERT_TRUE(std::equal(std::cbegin(ts), std::cend(ts), std::begin(expected)));
}