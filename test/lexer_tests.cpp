#include "obj-cpp/lexer.hpp"

#include "gtest/gtest.h"

#include <algorithm>
#include <iostream>

using namespace obj;

void print(const std::span<const Token> tokens)
{
    for (const auto& t : tokens)
        std::cout << t << '\n';
}

using SingleLineResult = std::vector<Token>;
using MultiLineResult  = std::vector<std::vector<Token>>;

[[nodiscard]] SingleLineResult lexer_single_line(const std::string& s)
{
    SingleLineResult line;
    const auto       end = lex_until_linefeed(std::data(s), line);
    assert(end[0] == '\0');
    return line;
}

[[nodiscard]] MultiLineResult lexer_multi_line(const std::string& s)
{
    MultiLineResult lines;
    for (auto p = std::data(s); p != (std::data(s) + std::size(s));)
    {
        std::vector<Token> line;
        p = lex_until_linefeed(p, line);
        lines.emplace_back(line);
    }
    return lines;
}

inline void assert_lexer_result(
    const MultiLineResult& result, const MultiLineResult& expected)
{
    ASSERT_EQ(std::size(result), std::size(expected));
    for (auto i = 0; i < std::size(result); ++i)
    {
        ASSERT_EQ(std::size(result[i]), std::size(expected[i]));
        ASSERT_EQ(result[i], expected[i]);
    }
}

GTEST_TEST(Lexer, SingleLineComment)
{
    const std::string      source   = "# this is a single line comment";
    const SingleLineResult expected = {};

    const auto result = lexer_single_line(source);
    ASSERT_EQ(result, expected);
}

GTEST_TEST(Lexer, Token)
{
    const std::string      source   = "abcde";
    const SingleLineResult expected = { "abcde" };

    const auto result = lexer_single_line(source);
    std::cout << source << '\n';
    for (const auto& t : result)
        std::cout << t << '\n';
    ASSERT_EQ(result, expected);
}

GTEST_TEST(Lexer, AlphabetCoverage)
{
    const std::string      source   = "f 1// 2/3/ 4//5 6/7/8";
    const SingleLineResult expected = { "f", "1//", "2/3/", "4//5", "6/7/8" };

    const auto result = lexer_single_line(source);
    std::cout << source << '\n';
    for (const auto& t : result)
        std::cout << t << '\n';
    ASSERT_EQ(result, expected);
}

GTEST_TEST(Lexer, MultiLineComments)
{
    const std::string source = "   # this is\n"
                               "v  # a comment\n"
                               "vn # on multiple lines";
    const MultiLineResult expected = {
        {}, // empty line
        { "v" },
        { "vn" },
    };

    const auto ts = lexer_multi_line(source);
    for (auto& l : ts)
        print(l);
    ASSERT_EQ(ts, expected);
}

GTEST_TEST(Lexer, MultiComments)
{
    const std::string source = "# this is a comment\n"
                               "v 3.14 0.46 9.10  # this is a comment too\n"
                               "v 2.13 1.98 5.23  # yet another #one\n";
    const MultiLineResult expected = {
        {}, // empty line
        { "v", "3.14", "0.46", "9.10" },
        { "v", "2.13", "1.98", "5.23" },
    };

    const auto result = lexer_multi_line(source);
    assert_lexer_result(result, expected);
}

GTEST_TEST(Lexer, SingleLineAlphanum)
{
    const std::string source = "v 1.2345 1.2345 1.2345";

    const SingleLineResult expected = { "v", "1.2345", "1.2345", "1.2345" };

    const auto result = lexer_single_line(source);
    print(result);
    ASSERT_EQ(result, expected);
}

GTEST_TEST(Lexer, MultiLineAlphanum)
{
    const std::string source = "aa bb\n"
                               "1.2345 1.2345";
    const MultiLineResult expected = {
        { "aa", "bb" },
        { "1.2345", "1.2345" },
    };

    const auto result = lexer_multi_line(source);
    assert_lexer_result(result, expected);
}

GTEST_TEST(Lexer, MixedTokenTypes)
{
    const std::string source = "aa bb\n"
                               "# comment line # still a comment\n"
                               "cc 123\n";
    const MultiLineResult expected = {
        { "aa", "bb" },
        {},
        { "cc", "123" },
    };

    const auto result = lexer_multi_line(source);
    assert_lexer_result(result, expected);
}