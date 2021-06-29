#include "lexer.hpp"
#include "object.hpp"
#include "parser.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

namespace obj
{
    using _pec = ParserErrorCode;

    // TODO: substitute constexpr with constinit when is available.

    constexpr const Value default_vertex_weight  = 1.f; // default value for vertex weight (from obj specs)
    constexpr const Value default_texcoord_value = 0.f; // default value for texture coordinate (from obj specs)
    constexpr const char  line_continuation      = '\\';
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

    // parse an index integer according to specs
    [[nodiscard]] Index _parse_index(const char* first, const char* last)
    {
        Index v{};
        if (const auto r = std::from_chars(first, last, v); r.ec != std::errc{})
            throw ParserError{ "Cannot parse " + std::string(first, last) + " as index" };
        //throw ParserError{ ParserErrorCode::invalid_arg_format };
        return v;
    }

    // parse an index integer according to specs
    [[nodiscard]] Index _parse_index(const Token& t)
    {
        return _parse_index(t.first(), t.last());
    }

    [[nodiscard]] Value _parse_value(const char* first, const char* last)
    {
        Value v{};
        if (const auto r = std::from_chars(first, last, v, std::chars_format::fixed);
            r.ec != std::errc{})
            throw ParserError{ "Cannot parse " + std::string(first, last) + " as value" };
        //throw ParserError{ ParserErrorCode::invalid_arg_format };
        return v;
    }

    // parse a floating point value according to specs
    [[nodiscard]] Value _parse_value(const Token& t)
    {
        return _parse_value(t.first(), t.last());
    }

    // starting from <first>, find the end of next line
    // skipping comments ('#') and line continuation ('\')
    [[nodiscard]] char* _next_line_end(char* first, char* last) noexcept
    {
        char* ch = first;
        for (;; ++ch)
        {
            while (*ch != '\n')
                ++ch;

            if (*first == '#' && ch != last) // line is a comment
                continue;
            if (ch[-1] == '\\' && ch != last) // line wraps into the next one
                continue;
        }
        return ch;
    }

    [[nodiscard]] Triplet _parse_triplet(const Token& t)
    {
        const auto s = t.view();

        const auto sep_1 = s.find_first_of('/');
        const auto v     = _parse_index(&s[0], &s[sep_1]);

        const auto sep_2 = s.find_first_of('/', sep_1);
        const auto vt    = _parse_index(&s[sep_1 + 1], &s[sep_2]);

        const auto vn = _parse_index(&s[sep_2 + 1], &s[s.size()]);
        return { .v = v, .vt = vt, .vn = vn };
    }

    void _handle_v_line(std::span<const Token> args, ParserResult& pr)
    {
        if (const auto s = std::size(args); s != 3 && s != 4)
            throw ParserError{ _pec::tag_v_invalid_args_count };

        Value v[4] = { 0, 0, 0, default_vertex_weight };
        for (auto i = 0; i < std::size(args); ++i)
            v[i] = _parse_value(args[i]);

        pr.data.v.emplace_back(v[0], v[1], v[2], v[3]);
    }

    void _handle_v_line_ext(std::span<const Token> args, ParserResult& pr);

    void _handle_vn_line(std::span<const Token> args, ParserResult& pr)
    {
        if (std::size(args) != 3)
            throw ParserError{ ParserErrorCode::tag_vn_invalid_args_count };

        Value vn[3] = { 0, 0, 0 };
        for (auto i = 0; i < 3; ++i)
            vn[i] = _parse_value(args[i]);

        pr.data.vn.emplace_back(vn[0], vn[1], vn[2]);
    }

    void _handle_vt_line(std::span<const Token> args, ParserResult& pr)
    {
        if (const auto s = std::size(args); s < 1 || s > 3)
            throw ParserError{ _pec::tag_vt_invalid_args_count };

        float vt[3] = { default_texcoord_value, default_texcoord_value, default_texcoord_value };
        for (auto i = 0; i < std::size(args); ++i)
            vt[i] = _parse_value(args[i]);

        pr.data.vt.emplace_back(vt[0], vt[1], vt[2]);
    }

    void _handle_f_line(std::span<const Token> args, ParserResult& pr)
    {
        if (std::size(args) != 3) // NOTE: currently we only support triangular faces
            throw ParserError{ _pec::tag_f_invalid_args_count };

        Face f{
            _parse_triplet(args[0]),
            _parse_triplet(args[1]),
            _parse_triplet(args[2])
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

    /* void _handle_o_line(std::span<const Token> args, ParserResult& pr)
    {
        if (std::size(args) != 1)
            throw ParserError{ _pec::tag_o_invalid_args_count };

        pr.objects.data.emplace_back();
        pr.objects.names.emplace_back(args[0].view());
    }*/

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

    [[nodiscard]] ParserResult parse(std::span<char> source, const ParserConfig& config)
    {
        const auto tokens = lex(source);

        ParserResult result;
        for (auto begin = std::cbegin(tokens); begin != std::cend(tokens);)
        {
            auto end = std::find_if(begin, std::cend(tokens),
                [](auto t) { return t.type() == Token::Type::endline; });

            if (end == begin) // skip empty line
            {
                std::advance(begin, 1);
                continue;
            }

            const auto tag = (*begin).view();
            assert(tag != "\n");

            // skip first token (the tag) and last token (the line terminator)
            const std::span<const Token> args{ std::next(begin), end };

            begin = std::next(end);
            if (_is_ignored_keyword(tag))
                continue;
            if (tag == "v")
            {
                _handle_v_line(args, result);
                continue;
            }
            else if (tag == "vn")
            {
                _handle_vn_line(args, result);
                continue;
            }
            else if (tag == "vt")
            {
                _handle_vt_line(args, result);
                continue;
            }
            else if (tag == "f")
            {
                _handle_f_line(args, result);
                continue;
            }
            else
                throw ParserError{ ParserErrorCode::unknown_tag };
        }
        return result;
    }


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

    // parse line with starting keyword 'v' and [first, last) tokens
    [[nodiscard]] parser_error_report
    _handle_v_line(const _line& l, std::span<const _token> args, _context& state) noexcept
    {
        const auto V_MIN_ARGC = 3;
        const auto V_MAX_ARGC = 4;

        if (std::size(args) < V_MIN_ARGC)
            return _make_token_error(l, args.front(), _pec::tag_v_invalid_args_count);
        if (std::size(args) > V_MAX_ARGC)
            return _make_token_error(l, args[V_MAX_ARGC + 1], _pec::tag_v_invalid_args_count);

        std::array<float, 4> v;
        v[3] = default_vertex_weight;
        for (auto i = 0; i < std::size(args); ++i)
        {
            if (const auto [ptr, errc] = std::from_chars(args[i].first(), args[i].last(), v[i]);
                errc != std::errc{})
            {
                return _make_token_error(l, args[i], _pec::invalid_arg_format);
            }
        }
        state.v.emplace_back(v[0], v[1], v[2], v[3]);
    }

    // parse line with starting keyword 'vn' and [first, last) tokens
    [[nodiscard]] parser_error_report
    _handle_vn_line(const _line& l, std::span<const _token> args, _context& state) noexcept
    {
        const auto VN_ARGC = 3;
        // NOTE: normal vector is not required to be normalized by .OBJ specs
        if (std::size(tokens) != VN_ARGC)
        {
            if (std::size(tokens) > VN_ARGC)
                return _make_token_error(line, tokens[VN_ARGC], _pec::tag_vn_invalid_args_count);
            else
                return _make_token_error(line, tokens.back(), _pec::tag_vn_invalid_args_count);
        }

        float xyz[VN_ARGC];
        for (auto i = 0; i < VN_ARGC; ++i)
        {
            if (const auto [ptr, err] = std::from_chars(tokens[i].begin, tokens[i].end, xyz[i]);
                err != std::errc{})
            {
                return _make_token_error(line, tokens[i], _pec::invalid_arg_format);
            }
        }
        for (auto coord : xyz)
            vn.emplace_back(coord);

        return {};
    }

    // parse line with starting keyword 'vt' and [first, last) tokens
    [[nodiscard]] std::optional<parser_error_report>
    _handle_vt_line(const _line& l, std::span<const _token> args, _context& state) noexcept
    {
        const auto VT_MIN_ARGC = 1;
        const auto VT_MAX_ARGC = 3;

        if (std::size(tokens) < VT_MIN_ARGC)
            return _make_token_error(line, tokens.back(), _pec::tag_vt_invalid_args_count);

        if (std::size(tokens) > VT_MAX_ARGC)
            return _make_token_error(line, tokens[VT_MAX_ARGC + 1], _pec::tag_vt_invalid_args_count);

        float texcoords[VT_MAX_ARGC];
        std::fill(std::begin(texcoords), std::end(texcoords), DEFAULT_VERTEX_TEXCOORD);
        for (auto i = 0; i < std::size(tokens); ++i)
        {
            if (const auto [ptr, err] = std::from_chars(tokens[i].begin, tokens[i].end, texcoords[i]);
                err != std::errc{})
            {
                return _make_token_error(line, tokens[i], _pec::invalid_arg_format);
            }
        }
        for (auto coord : texcoords)
            vt.emplace_back(coord);

        return {};
    }

    // parse line with starting keyword 'f' and [first, last) tokens
    [[nodiscard]] parser_error_report
    _handle_f_line(const _line& l, std::span<const _token> args, _context& state) noexcept
    {
        using _pec = ParserErrorCode;

        DRAKO_ASSERT(!std::empty(tokens));
        DRAKO_ASSERT(tokens.front().view() == "f");

        const auto F_MIN_ARGC = 4;

        if (std::size(tokens) < F_MIN_ARGC)
            return _make_token_error(line, tokens.back(), _pec::tag_f_invalid_args_count);

        std::vector<int32_t> v(std::size(tokens));  // geometric vertices index
        std::vector<int32_t> vt(std::size(tokens)); // texture vertices index
        std::vector<int32_t> vn(std::size(tokens)); // vertex normals index

        for (auto i = 1 /*skip keyword token*/; i < std::size(tokens); ++i)
        {
            // tokenize triplet v/[vt]/[vn]
            if (std::count(tokens[i].begin, tokens[i].end, '/') != 2)
                return _make_token_error(line, tokens[i], _pec::tag_f_invalid_args_format);

            _parser_token tk_v, tk_vt, tk_vn;
            {
                auto c = tokens[i].begin;
                while (*c != '/')
                    ++c;
                tk_v = _parser_token{ tokens[i].begin, c };

                while (*c != '/')
                    ++c;
                tk_vt = _parser_token{ tk_v.end + 1, c };

                tk_vn = _parser_token{ tk_vt.end + 1, tokens[i].end };
            }

            int32_t index_v, index_vt, index_vn;

            /*vvv [v/../..] vvv*/
            if (const auto [ptr, ec] = std::from_chars(tk_v.begin, tk_v.end, index_v);
                ec != std::errc{})
            {
                return _make_token_error(line, tokens[i], _pec::tag_f_invalid_args_format);
            }
            if (const auto i = index_v; i == 0 || std::abs(i) > std::size(obj.vertex_points))
            {
                return _make_token_error(line, tokens[i], _pec::index_out_of_range);
            }

            /*vvv [../vt/..] vvv*/
            if (std::distance(tk_vt.begin, tk_vt.end) > 0)
            { // vt token is present
                if (const auto [ptr, ec] = std::from_chars(tk_vt.begin, tk_vt.end, index_vt);
                    ec != std::errc{})
                {
                    return _make_token_error(line, tokens[i], _pec::tag_f_invalid_args_format);
                }
                if (const auto i = index_vt; i == 0 || std::abs(i) > std::size(obj.vertex_texcoords))
                {
                    return _make_token_error(line, tokens[i], _pec::invalid_arg_format);
                }
            }

            /*vvv [../../vn] vvv*/
            if (std::distance(tk_vn.begin, tk_vn.end) > 0)
            { // vn token is present
                if (const auto [ptr, ec] = std::from_chars(tk_vn.begin, tk_vn.end, index_vn);
                    ec != std::errc{})
                {
                    return _make_token_error(line, tokens[i], _pec::tag_f_invalid_args_format);
                }
                if (const auto i = index_vn; i == 0 || std::abs(i) > std::size(obj.vertex_normals))
                {
                    return _make_token_error(line, tokens[i], _pec::invalid_arg_format);
                }
            }
            obj.faces.emplace_back(index_v);
            obj.faces.emplace_back(index_vt);
            obj.faces.emplace_back(index_vn);
        }
        return {};
    }

    // parse line with starting keyword 'o' and [first, last) tokens
    [[nodiscard]] parser_error_report
    _handle_o_line(std::span<const _token> args, _context& state) noexcept
    {
        static_assert(std::is_same_v<std::iterator_traits<Iter>::value_type, _parser_token>,
            "Iterator value_type must match " DRAKO_STRINGIZE(_parser_token));

        if (std::distance(_in_first_token, _in_last_token) != 1)
            return _make_token_error(std::next(_in_first_token), _pec::tag_o_invalid_args_count);

        _out_obj = { std::string{ _in_first_token.begin, _in_last_token.end } };
        continue;
    }

    [[nodiscard]] std::variant<parser_result, parser_error_report>
    _try_parse_buffer_until_error(const std::string_view source, const _config& config)
    {
        _prr result{};

        std::vector<float>    v_points(_in_config.expected_vertex_count * 3);    // vertex geometry
        std::vector<float>    v_normals(_in_config.expected_vertex_count * 3);   // vertex normals
        std::vector<float>    v_texcoords(_in_config.expected_vertex_count * 3); // vertex texcoords
        std::vector<uint32_t> faces(_in_config.expected_triangle_count * 3);

        object            curr_object;
        std::size_t       curr_source_row = 0;
        std::vector<_ptk> tokens(10); // estimate of max token count
        for (const auto line_begin = _in_src_begin; line_begin != _in_src_end;)
        {
            tokens.clear();
            /*
                do
                {
                    ++curr_source_row;

                    const auto line_end = std::find(line_begin, _in_src_end, '\n');
                    for (auto buffer_begin = line_begin; buffer_begin != line_end;)
                    {
                        // skip leading whitespace
                        const auto token_begin = std::find_if(buffer_begin, line_end,
                            [](auto c) { return !std::isspace(static_cast<unsigned char>(c)); });

                        // reach trailing whitespace or line termination
                        const auto token_end = std::find_if(token_begin, line_end,
                            [](auto c) { return std::isspace(static_cast<unsigned char>(c)); });

                        const auto col = static_cast<size_t>(std::distance(line_begin, token_begin));
                        tokens.emplace_back(_ptk{ token_begin, token_end, curr_source_row, col });

                        buffer_begin = token_end;
                    }
                } while (std::string_view{ tokens.back() } == line_continuation);
                */
            _tokenize_line()

                const std::string_view keyword{ tokens.front() };
            if (keyword == "v") // vertex geometric data: v x y z [w]
            {
                if (const auto err = _parse_v_line(tokens, v_points); err)
                    return *err;
                continue;
            }
            else if (keyword == "vn") // vertex normal data: vn x y z
            {
                if (const auto err = _parse_vn_line(tokens, v_normals); err)
                    return *err;
                continue;
            }
            else if (keyword == "vt") // vertex uv data: vt u [v] [w]
            {
                if (const auto err = _parse_vt_line(tokens, v_texcoords); err)
                    return *err;
                continue;
            }
            else if (keyword == "f") // face element: v1/[vt1]/[vn1] v2/[vt2]/[vn2] v3/[vt3]/[vn3] [v4/vt4/vn4 ...]
            {
                if (const auto err = _parse_f_line(tokens, curr_object); err)
                    return *err;
                continue;
            }
            else if (keyword == "o") // new object name statement: o object_name
            {
                if (std::size(tokens) != 2)
                    return _make_token_error(tokens.front(), _pec::tag_o_invalid_args_count);

                result.objects.emplace_back(curr_object);
                curr_object = object{ std::string{ tokens[0].begin, tokens[0].end } };
                continue;
            }

            if (_is_ignored_keyword(keyword))
                continue; // skip current line

            return _make_token_error(tokens.front(), _pec::unknown_tag); // unrecognized keyword
        }

        return result;
    }

    [[nodiscard]] std::tuple<parser_result, std::vector<parser_error_report>>
    _try_parse_buffer_skip_errors(const std::string_view source, const _config& config) noexcept
    {
        // TODO: review impl
        if (src_begin == src_end)
            return { {}, {} };

        std::vector<parser_error_report> errors; // logs error generated by parsing
        std::vector<object>              objects;
        std::vector<float>               v_points(cfg.expected_vertex_count * 3);    // vertex geometry
        std::vector<float>               v_normals(cfg.expected_vertex_count * 3);   // vertex normals
        std::vector<float>               v_texcoords(cfg.expected_vertex_count * 3); // vertex texcoords
        std::vector<uint32_t>            faces(cfg.expected_triangle_count * 3);

        object            curr_object;
        std::size_t       curr_line_index = 1;
        std::vector<_ptk> tokens(100);
        for (const auto line_begin = src_begin; line_begin != src_end;)
        {
            _tokenize_line(line_begin, line_end, tokens);

            const std::string_view keyword{ tokens.front() };
            if (keyword == "v") // vertex geometric data: v x y z [w]
            {
                if (const auto err = _parse_v_line(tokens, v_points); err)
                    errors.emplace_back(*err);
                continue;
            }
            else if (keyword == "vn") // vertex normal data: vn x y z
            {
                if (const auto err = _parse_vn_line(tokens, v_normals); err)
                    errors.emplace_back(*err);
                continue;
            }
            else if (keyword == "vt") // vertex uv data: vt u [v] [w]
            {
                if (const auto err = _parse_vt_line(tokens, v_texcoords); err)
                    errors.emplace_back(*err);
                continue;
            }
            else if (keyword == "f") // face element: v1/[vt1]/[vn1] v2/[vt2]/[vn2] v3/[vt3]/[vn3] [v4/vt4/vn4 ...]
            {
                if (const auto err = _parse_f_line(tokens, curr_object); err)
                    errors.emplace_back(*err);
                continue;
            }
            else if (keyword == "o") // new object name statement: o object_name
            {
                if (std::size(tokens) == 2)
                {
                    objects.emplace_back(curr_object);
                    curr_object = object{ std::string{ tokens[0].begin, tokens[0].end } };
                }
                else
                    errors.emplace_back(_make_token_error(tokens.front(), _pec::tag_o_invalid_args_count));

                continue;
            }

            if (_is_ignored_keyword(keyword))
                continue; // skip current line

            errors.emplace_back(_make_token_error(tokens.front(), _pec::unknown_tag)); // unrecognized keyword
        }
        return { objects, errors };
    }


    [[nodiscard]] std::variant<parser_result, parser_error_report>
    try_parse(const std::string_view source, const ParserConfig& config) noexcept
    {
        return _try_parse_buffer_until_error(source.data(), source.data() + std::size(source), config);
    }

    [[nodiscard]] std::tuple<parser_result, std::vector<parser_error_report>>
    try_force_parse(const std::string_view source, const ParserConfig& config) noexcept
    {
        return _try_parse_buffer_skip_errors(source.data(), source.data() + std::size(source), config);
    }

#endif /*^^^ error codes ^^^*/

} // namespace obj