#pragma once
#ifndef OBJCPP_PARSER_HPP
#define OBJCPP_PARSER_HPP

#include "obj-cpp/core.hpp"
#include "obj-cpp/lexer.hpp"

#include <charconv>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#if __cpp_lib_string_view
#include <string_view>
#endif

#if __cpp_lib_span
#include <span>
#endif

namespace obj
{
    enum class ParserErrorCode
    {
        duplicate_object_name,
        index_out_of_range,
        invalid_arg_count,
        invalid_arg_format,
        tag_f_invalid_args_count,
        tag_f_invalid_args_format,
        tag_o_invalid_args_count,
        tag_v_invalid_args_count,
        tag_vn_invalid_args_count,
        tag_vt_invalid_args_count,
        unknown_tag
    };

    [[nodiscard]] inline std::string to_string(ParserErrorCode ec) noexcept
    {
        using _pec = ParserErrorCode;
        switch (ec)
        {
            case _pec::duplicate_object_name: return "Multiple object names.";
            case _pec::invalid_arg_count: return "Invalid arguments count.";
            case _pec::invalid_arg_format: return "Invalid argument format.";
            case _pec::index_out_of_range: return "Index out of valid range.";
            case _pec::tag_o_invalid_args_count: return "Tag 'o' requires one argument.";
            case _pec::tag_v_invalid_args_count: return "Tag 'v' requires 3 or 4 arguments.";
            case _pec::tag_vn_invalid_args_count: return "Tag 'vn' requires 3 arguments.";
            case _pec::tag_vt_invalid_args_count: return "Tag 'vt' requires 2 or 3 arguments.";
            case _pec::tag_f_invalid_args_count: return "Tag 'f' requires 3 arguments.";
            case _pec::tag_f_invalid_args_format: return "Invalid triplet format.";
            case _pec::unknown_tag: return "Unknown tag.";
            default: return "Unknown error code.";
        }
    }


    /// @brief Exception class for parser errors.
    class ParserError : public std::runtime_error
    {
    public:
        explicit ParserError(ParserErrorCode ec)
            : std::runtime_error{ to_string(ec) } {}

        explicit ParserError(const char* msg)
            : std::runtime_error{ msg } {}

        explicit ParserError(const std::string& s)
            : std::runtime_error{ s } {}
    };

    class ParserTokenError : public ParserError
    {
    public:
        explicit ParserTokenError(ParserErrorCode ec)
            : ParserError{ ec } {}
    };



    /*
    enum class ParserWarningCode
    {
    };

    [[nodiscard]] inline std::string to_string(ParserWarningCodee c) noexcept
    {
        using _pwc = ParserWarningCode;
        switch (c)
        {
            default: return "Unknow warning code";
        }
    }

    /// @brief Report data of a parsing warning.
    ///
    struct ParserWarningReport
    {
        size_t              row; // index of the line that produced the warning
        size_t              col; // index of the token that produced the warning
        size_t              offset;
        parser_warning_code warc;

        constexpr explicit ParserWarningReport(
            size_t row, size_t col, size_t offset, parser_warning_code wc) noexcept
            : row{ row }, col{ col }, offset{ offset }, warc{ wc } {}
    };*/

    /// @brief Report data of a parsing error.
    class ParserErrorReport
    {
    public:
        explicit constexpr ParserErrorReport(
            std::size_t row, std::size_t col, std::size_t offset, ParserErrorCode ec) noexcept
            : _char_index{ offset }, _row{ row }, _col{ col }, _ec{ ec } {}

        /// @brief Index of the character in the buffer.
        ///
        [[nodiscard]] constexpr std::size_t
        char_index() const noexcept { return _char_index; }

        /// @brief Line number location of the error.
        ///
        [[nodiscard]] constexpr std::size_t
        line() const noexcept { return _row; }

        /// @brief Column number location of the error.
        ///
        [[nodiscard]] constexpr std::size_t
        column() const noexcept { return _col; }

        /// @brief Error code.
        ///
        [[nodiscard]] constexpr std::underlying_type_t<ParserErrorCode>
        value() const noexcept { return static_cast<std::underlying_type_t<ParserErrorCode>>(_ec); }

        /// @brief Short explanatory description of the error.
        ///
        [[nodiscard]] std::string
        message() const noexcept { return to_string(_ec); }

    private:
        std::size_t     _char_index; // [ 0, size(source) )  , zero-based
        std::size_t     _row;        // [ 1, #rows ]         , one-based
        std::size_t     _col;        // [ 1, size(col) ]     , one-based
        ParserErrorCode _ec;
    };

#if !defined(_drako_disable_exceptions) /*vvv exceptions vvv*/

    // parse an index integer according to specs
    //template <class Index>
    [[nodiscard]] inline Index parse_index(const char* first, const char* last)
    {
        Index v{};
        if (const auto r = std::from_chars(first, last, v); r.ec != std::errc{})
            throw ParserError{ "Cannot parse " + std::string(first, last) + " as index" };
        //throw ParserError{ ParserErrorCode::invalid_arg_format };
        return v;
    }

    // parse an index integer according to specs
    //template <class Index>
    [[nodiscard]] inline Index parse_index(const Token& t)
    {
        return parse_index(std::data(t), std::data(t) + std::size(t));
    }

    //template <class Value>
    [[nodiscard]] inline Value parse_value(const char* first, const char* last)
    {
        Value v{};
        if (const auto r = std::from_chars(first, last, v, std::chars_format::fixed);
            r.ec != std::errc{})
            throw ParserError{ "Cannot parse " + std::string(first, last) + " as value" };
        //throw ParserError{ ParserErrorCode::invalid_arg_format };
        return v;
    }

    // parse a floating point value according to specs
    //template <class Value>
    [[nodiscard]] inline Value parse_value(const Token& t)
    {
        return parse_value(std::data(t), std::data(t) + std::size(t));
    }

    //template <class Index>
    [[nodiscard]] inline Triplet parse_triplet(const Token& t)
    {
        assert(std::size(t) > 0);

        if (std::count(std::cbegin(t), std::cend(t), '/') != 2)
            throw ParserError{ ParserErrorCode::invalid_arg_format };

        Triplet triplet{};

        const auto beg_1 = 0;
        const auto end_1 = t.find_first_of('/');
        if (end_1 == beg_1) // there must be the vertex index at least
            throw ParserError{ ParserErrorCode::invalid_arg_format };
        triplet.v = parse_index(t.substr(beg_1, end_1));
        //triplet.v = parse_index(std::data(t), std::data(t) + end_1);

        const auto beg_2 = end_1 + 1;
        const auto end_2 = t.find_first_of('/', beg_2);
        if ((end_2 - end_1) > 1) // at least one character
            triplet.vt = parse_index(t.substr(beg_2, end_2 - beg_2));

        const auto beg_3 = end_2 + 1;
        const auto end_3 = std::size(t);
        if ((end_3 - end_2) > 1) // at least one character
            triplet.vn = parse_index(t.substr(beg_3, end_3 - beg_3));
        //triplet.vn = parse_index(std::data(t) + beg_3, std::data(t) + end_3);

        return triplet;
    }

#else /*^^^ exceptions ^^^/vvv error codes vvv*/


    /// @brief Parse an ASCII string according to OBJ format.
    ///
    /// Stops at first parsing error.
    ///
    [[nodiscard]] std::variant<ParserResult, ParserErrorReport>
    try_parse(const std::string_view source, const ParserConfig& config) noexcept;

    /// @brief Parse an ASCII string acoording to OBJ format.
    ///
    [[nodiscard]] std::tuple<ParserResult, std::vector<ParserErrorReport>>
    try_force_parse(const std::string_view source, const ParserConfig& config) noexcept;

#endif /*^^^ error codes ^^^*/

} // namespace obj

#endif // !OBJCPP_PARSER_HPP