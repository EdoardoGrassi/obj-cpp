#include "obj-cpp/obj_parser.hpp"

#include "obj-cpp/lexer.hpp"
#include "obj-cpp/parser.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#if __cpp_lib_string_view
#include <string_view>
#endif

#if __cpp_lib_span
#include <span>
#endif

namespace obj
{
    using _pec = ParserErrorCode;


    /// @brief Templated default values [from obj specs].
    template <class Value>
    struct Defaults
    {
        /// @brief Default value for vertex weight.
        static constexpr const Value vertex_weight = 1.f;

        /// @brief Default value for texture coordinate.
        static constexpr const Value texcoord_value = 0.f;
    };

    constexpr const char line_continuation = '\\';

    //constexpr const std::string_view OBJ_TAG_COMMENT         = "#";
    //constexpr const std::string_view OBJ_TAG_POINT           = "v";
    //constexpr const std::string_view OBJ_TAG_NORMAL          = "vn";
    //constexpr const std::string_view OBJ_TAG_TEXCOORDS       = "vt"

    constexpr const std::string_view ignored_keywords[] = {
        "call",   // file import command
        "csh",    // UNIX shell command
        "g",      // group statement
        "mtllib", // material attribute
        "o",      // object name statement
        "s",      // smoothing group statement
        "usemtl"  // material reference statement
    };

    /*
    struct _token // single token info
    {
        const char* _first;
        const char* _last;
        //size_t           row;
        //size_t           col;

        explicit constexpr _token(const char* first, const char* last) noexcept
            : _first{ first }, _last{ last }
        {
            assert(first < last); // at least one character
        }

        explicit constexpr _token(std::string_view token,
            size_t row, size_t column, size_t file_char_index) noexcept
            : _string{ token }, row{ row }, col{ column }, offset{ file_char_index }
        {
        }

        [[nodiscard]] const char* first() const noexcept { return _first; }
        [[nodiscard]] const char* last() const noexcept { return _last; }

        [[nodiscard]] std::string_view view() const noexcept
        {
            return { _first, static_cast<std::size_t>(std::distance(_first, _last)) };
        }
        //[[nodiscard]] std::size_t file_line() const noexcept { return row; }
        //[[nodiscard]] std::size_t file_column() const noexcept { return col; }
    };

    struct _line // single file line info
    {
        const char* begin;
        std::size_t row;
    };
    */

    // ckeck if character is considered whitespace
    // according to .obj format specs
    [[nodiscard]] bool _is_whitespace(char c) noexcept
    {
        return c == ' ' || c == '\t' || c == '\v';
    }

    // check if a keyword is ignored by current implementation
    [[nodiscard]] bool _is_ignored_keyword(const std::string_view keyword) noexcept
    {
        return std::find(std::cbegin(ignored_keywords), std::cend(ignored_keywords),
                   keyword) != std::cend(ignored_keywords);
    }


#if !defined(_drako_disable_exception) /*vvv exceptions vvv*/

    //template <class Value, class Index>
    void handle_v_line(std::span<const Token> args, ObjParserResult& pr)
    {
        if (const auto s = std::size(args); s != 3 && s != 4)
            throw ParserError{ _pec::tag_v_invalid_args_count };

        Value v[4] = { 0, 0, 0, Defaults<Value>::vertex_weight };
        for (auto i = 0; i < std::size(args); ++i)
            v[i] = parse_value(args[i]);

        pr.data.v.emplace_back(v[0], v[1], v[2], v[3]);
    }

    //template <class Value, class Index>
    //void handle_v_line_ext(std::span<const Token> args, ParserResult<Value, Index>& pr);

    //template <class Value, class Index>
    void handle_vn_line(std::span<const Token> args, ObjParserResult& pr)
    {
        if (std::size(args) != 3)
            throw ParserError{ ParserErrorCode::tag_vn_invalid_args_count };

        Value vn[3] = { 0, 0, 0 };
        for (auto i = 0; i < 3; ++i)
            vn[i] = parse_value(args[i]);

        pr.data.vn.emplace_back(vn[0], vn[1], vn[2]);
    }

    //template <class V, class I>
    void handle_vt_line(std::span<const Token> args, ObjParserResult& pr)
    {
        if (const auto s = std::size(args); s < 1 || s > 3)
            throw ParserError{ _pec::tag_vt_invalid_args_count };

        Value vt[3] = {
            Defaults<Value>::texcoord_value,
            Defaults<Value>::texcoord_value,
            Defaults<Value>::texcoord_value
        };
        for (auto i = 0; i < std::size(args); ++i)
            vt[i] = parse_value(args[i]);

        pr.data.vt.emplace_back(vt[0], vt[1], vt[2]);
    }

    //template <class V, class I>
    void handle_f_line(std::span<const Token> args, ObjParserResult& pr)
    {
        if (std::size(args) != 3) // NOTE: currently we only support triangular faces
            throw ParserError{ _pec::tag_f_invalid_args_count };

        Face f{
            parse_triplet(args[0]),
            parse_triplet(args[1]),
            parse_triplet(args[2])
        };
        //const auto v1, vt1, vn1 = _parse_triplette(args[0]);

        if (std::any_of(f.triplets.cbegin(), f.triplets.cend(),
                [](auto x) { return x.v == 0; }))
            throw ParserError{ _pec::tag_f_invalid_args_format };

        /*
        // check if some triple is missing a vertex
        if (const auto c = std::count(vt.begin(), vt.end(), 0); c != 0 && c != 3)
            throw ParserError{ _pec::tag_f_invalid_args_format };

        // check if some triple is missing a vertex
        if (const auto c = std::count(vn.begin(), vn.end(), 0); c != 0 && c != 3)
            throw ParserError{ _pec::tag_f_invalid_args_format };
            */

        pr.data.faces.emplace_back(f);
    }

    //template <class V, class I>
    void handle_o_line(std::span<const Token> args, ObjParserResult& pr)
    {
        if (std::size(args) != 1)
            throw ParserError{ _pec::tag_o_invalid_args_count };

        const auto& name = args[0];
        if (std::any_of(std::cbegin(pr.objects), std::cend(pr.objects),
                [&](const auto& x) { return x.name == name; }))
            throw ParserError{ _pec::duplicate_object_name };

        pr.objects.push_back({ std::string{ name }, {} });
    }

    /* void _handle_g_line(std::span<const Token> args, _context& state)
    {
        if (std::size(args) == 0)
            throw ParserError{ _pec::invalid_arg_count };

        assert(state.last_face_index_checkpoint <= std::size(state.faces));

        auto& data = state.groups.data;
        for (auto g = 0; g < std::size(state.active_groups_ids); ++g)
            for (auto i = state.last_face_index_checkpoint; i < std::size(state.faces); ++i)
                data[g].faces.emplace_back(i);


        state.active_groups_ids.clear();
        state.last_face_index_checkpoint = std::size(state.faces);

        auto& names = state.groups.names;
        for (const auto& name : args)
        {
            const auto it = std::find(std::cbegin(names), std::cend(names), name.view());
            if (it == std::cend(names))
            { // create a new group with provided name
                names.emplace_back(name.view());
                state.groups.data.emplace_back();
            }
            const auto index = static_cast<std::size_t>(std::distance(std::cbegin(names), it));
            state.active_groups_ids.emplace_back(index);
        }
    }*/

    //template <class V, class I>
    ObjParserResult _parse_as_obj_impl(
        const char* data, const std::size_t size, const ObjParserConfig& c)
    {
        using Handler = void (*)(std::span<const Token>, ObjParserResult&);

        // matched tags and specialized grammar parsing functions
        const std::vector<std::pair<Token, Handler>> tag_fun_pairs = {
            { "v", handle_v_line },
            { "vn", handle_vn_line },
            { "vt", handle_vt_line },
            { "f", handle_f_line },
            { "o", handle_o_line },
            //{ "mtllib", nullptr },
        };

        ObjParserResult result;

        std::vector<Token> tokens;
        tokens.reserve(64);
        for (auto lexer_position = data; lexer_position != data + size;)
        {
            // extract tokens from the next line
            lexer_position = lex_until_linefeed(lexer_position, tokens);
            if (std::empty(tokens))
                continue;

            const auto& tag = tokens[0];
            if (const auto it = std::find_if(std::cbegin(tag_fun_pairs), std::cend(tag_fun_pairs),
                    [&](const auto& x) { return x.first == tag; });
                it != std::cend(tag_fun_pairs))
            {
                const auto args = std::span{ tokens }.last(std::size(tokens) - 1);
                std::invoke((*it).second, args, result);
            }
            else
            {
                if (std::none_of(std::cbegin(ignored_keywords), std::cend(ignored_keywords),
                        [&](const auto& x) { return x == tag; }))
                    throw ParserError{ ParserErrorCode::unknown_tag };
            }
            tokens.clear();
        }
        return result;
    }

    ObjParserResult parse_as_obj(
        const std::string& s, const ObjParserConfig& c)
    {
        return _parse_as_obj_impl(std::data(s), std::size(s), c);
    }

#if __cpp_lib_string_view
    ObjParserResult parse_as_obj(
        const std::string_view s, const ObjParserConfig& c)
    {
        return _parse_as_obj_impl(std::data(s), std::size(s), c);
    }
#endif

#if __cpp_lib_span
    ObjParserResult parse_as_obj(
        const std::span<const char> s, const ObjParserConfig& c)
    {
        assert(s.back() == '\0'); // the buffer must be null-terminated like C strings
        return _parse_as_obj_impl(std::data(s), std::size(s), c);
    }
#endif


#else /*^^^ exceptions ^^^/vvv error codes vvv*/

    // construct report for line wide errors
    [[nodiscard]] parser_error_report _make_line_error(const _line& l, _pec ec) noexcept
    {
        return { l.row, 0, 0, ec };
    }

    // construct report for token related errors
    [[nodiscard]] parser_error_report _make_token_error(const _token& t, _pec ec) noexcept
    {
        return { t.row, t.col, 0, ec };
    }

    // construct report for token related errors
    [[nodiscard]] parser_error_report
    _make_token_error(const _line& l, const _token& t, _pec ec) noexcept
    {
        const auto col = static_cast<size_t>(std::distance(l.begin, t.begin));
        return parser_error_report{ l.row, col, 0, ec };
    }

#endif /*^^^ error codes ^^^*/

} // namespace obj