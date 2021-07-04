#pragma once
#ifndef OBJCPP_PARSER_HPP
#define OBJCPP_PARSER_HPP

#include "obj-cpp/core.hpp"

#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

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


    /// @brief Configuration parameters for the parser.
    struct ParserConfig
    {
        /// @brief Enable specific extensions to .obj standard.
        enum class ExtensionFlag
        {
            standard = 0,
            vertex_color = (1 << 0),
        };

        /// @brief Expected number of objects.
        ///
        std::size_t expected_object_count = 1;

        /// @brief Expected number of vertices.
        ///
        std::size_t expected_vertex_count = 10'000;

        /// @brief Expected number of triangle.
        ///
        std::size_t expected_triangle_count = 10'000;

        ExtensionFlag flags = ExtensionFlag::standard;
    };


    /// @brief Result of parsing of an .obj file.
    struct ParserResult
    {
        MeshData data;
        //std::vector<Object> objects;
        //std::vector<Group>  groups;
    };

    /*
    enum class parser_warning_code
    {
    };

    [[nodiscard]] inline std::string to_string(parser_warning_code wc) noexcept
    {
        using _pwc = parser_warning_code;
        switch (wc)
        {
            default: return "Unknow warning code";
        }
    }

    /// @brief Report data of a parsing warning.
    ///
    struct parser_warning_report
    {
        size_t              row; // index of the line that produced the warning
        size_t              col; // index of the token that produced the warning
        size_t              offset;
        parser_warning_code warc;

        constexpr explicit parser_warning_report(
            size_t row, size_t col, size_t offset, parser_warning_code wc) noexcept
            : row{ row }, col{ col }, offset{ offset }, warc{ wc } {}
    };*/

    /// @brief Report data of a parsing error.
    class ParserErrorReport
    {
    public:
        explicit constexpr ParserErrorReport(
            size_t row, size_t col, size_t offset, ParserErrorCode ec) noexcept
            : _char_index{ offset }, _row{ row }, _col{ col }, _ec{ ec } {}

        /// @brief Index of the character in the buffer.
        ///
        [[nodiscard]] constexpr size_t
            char_index() const noexcept { return _char_index; }

        /// @brief Line number location of the error.
        ///
        [[nodiscard]] constexpr size_t
            line() const noexcept { return _row; }

        /// @brief Column number location of the error.
        ///
        [[nodiscard]] constexpr size_t
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


    class Parser
    {
    public:
        explicit Parser(const ParserConfig& c, const std::size_t buffer_size)
            : _config{ c }, _size{ buffer_size }, _buffer{ std::make_unique<std::byte[]>(_size) } {}

        //[[nodiscard]] ParserResult parse_file();
        [[nodiscard]] ParserResult parse(const std::string_view s);

    private:
        ParserConfig                 _config;
        std::size_t                  _size;
        std::unique_ptr<std::byte[]> _buffer;
    };


#if !defined(_drako_disable_exceptions) /*vvv exceptions vvv*/

    [[nodiscard]] ParserResult parse(std::span<char> s, const ParserConfig& c);

    //[[nodiscard]] ParserResult parse(std::string_view s, const ParserConfig& c);

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