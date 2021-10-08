#include "obj-cpp/mtl_parser.hpp"

#include "obj-cpp/lexer.hpp"
#include "obj-cpp/parser.hpp"

#include <cassert>
#include <map>
#include <span>

namespace obj
{
    //template <class V>
    void handle_newmtl(const std::span<const Token> args, MtlParserResult& r)
    {
        if (std::size(args) != 1)
            throw ParserError{ ParserErrorCode::invalid_arg_count };

        r.materials.push_back({ .name = std::string{ args[0] } });
    }

    //template <class V>
    void handle_ka(const std::span<const Token> args, MtlParserResult& r)
    {
        if (std::size(args) != 3)
            throw ParserError{ ParserErrorCode::invalid_arg_count };

        r.materials.back().ka[0] = parse_value(args[0]);
        r.materials.back().ka[1] = parse_value(args[1]);
        r.materials.back().ka[2] = parse_value(args[2]);
    }

    //template <class V>
    void handle_kd(const std::span<const Token> args, MtlParserResult& r)
    {
        if (std::size(args) != 3)
            throw ParserError{ ParserErrorCode::invalid_arg_count };

        r.materials.back().kd[0] = parse_value(args[0]);
        r.materials.back().kd[1] = parse_value(args[1]);
        r.materials.back().kd[2] = parse_value(args[2]);
    }

    //template <class V>
    void handle_ks(const std::span<const Token> args, MtlParserResult& r)
    {
        if (std::size(args) != 3)
            throw ParserError{ ParserErrorCode::invalid_arg_count };

        r.materials.back().ks[0] = parse_value(args[0]);
        r.materials.back().ks[1] = parse_value(args[1]);
        r.materials.back().ks[2] = parse_value(args[2]);
    }

    MtlParserResult _parse_as_mtl_impl(
        const char* data, const std::size_t size, const MtlParserConfig& c)
    {
        using Handler = void (*)(const std::span<const Token>, MtlParserResult&);

        static const std::map<Token, Handler> tag_to_fun = {
            { "newmtl", handle_newmtl },
            { "Ka", handle_ka },
            { "Kd", handle_kd },
            { "Ks", handle_ks },
        };

        MtlParserResult result;
        for (auto pos = data; pos != data + size;)
        {
            std::vector<Token> tokens;
            pos = lex_until_linefeed(pos, tokens);

            if (std::empty(tokens))
                continue;

            const auto& tag  = tokens[0];
            const auto& args = std::span{ tokens }.last(std::size(tokens) - 1);
            if (const auto fun = tag_to_fun.find(tag); fun != std::cend(tag_to_fun))
                std::invoke((*fun).second, args, result);
            else
                throw ParserError{ ParserErrorCode::unknown_tag };
        }

        return result;
    }

    MtlParserResult parse_as_mtl(
        const std::string& s, const MtlParserConfig& c)
    {
        return _parse_as_mtl_impl(std::data(s), std::size(s), c);
    }

#if __cpp_lib_string_view
    MtlParserResult parse_as_mtl(
        const std::string_view s, const MtlParserConfig& c)
    {
        return _parse_as_mtl_impl(std::data(s), std::size(s), c);
    }
#endif

#if __cpp_lib_span
    MtlParserResult parse_as_mtl(
        const std::span<const char> s, const MtlParserConfig& c)
    {
        assert(s.back() == '\0'); // the buffer must be null-terminated like C strings
        return _parse_as_mtl_impl(std::data(s), std::size(s), c);
    }
#endif

} // namespace obj