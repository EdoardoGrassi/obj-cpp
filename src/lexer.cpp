#include "obj-cpp/lexer.hpp"

#include <algorithm>
#include <cassert>
#include <span>
#include <stdexcept>
#include <vector>

namespace obj
{
    [[nodiscard]] std::vector<Token> lex(std::span<const char> s)
    {
        using _fsa   = FiniteStateAutomata;
        using _state = _fsa::State;
        //using _ttype = Token::Type;

        const static _fsa fsa{}; // TODO: create lexer objects on the fly / make users do it?
        //print_table(fsa);

        assert(std::size(s) > 0);
        assert(s.back() == '\0'); // input buffer end mark

        std::vector<Token> tokens;
        tokens.reserve(1024);
        for (auto pos = std::data(s);;)
        {
            auto state = fsa.start_state();
            // skip whitespace and comments
            for (; fsa.skip_state(state); ++pos)
                state = fsa.advance(state, static_cast<unsigned char>(*pos));

            auto begin = pos - 1;
            for (; !fsa.final_state(state); ++pos)
            {
                assert(!fsa.skip_state(state));
                state = fsa.advance(state, static_cast<unsigned char>(*pos));
            }
            //--pos; // backtrack last character

            switch (state)
            {
                case _state::final_alphanum:
                    --pos; // rollback last character
                    //tokens.emplace_back(_ttype::alphanum, begin, pos);
                    tokens.push_back({ begin, pos });
                    break;

                case _state::final_newline:
                    //tokens.push_back({ .first = begin, .last = pos, .type = _type::endline });
                    //tokens.emplace_back(_ttype::endline, begin, pos);
                    break;

                case _state::final_input_end: // reached end of input
                    return tokens;

                case _state::final_error:
                    throw std::runtime_error{ "Unexpected character while in state " + to_string(state) };

                default:
                    throw std::logic_error{ "Unexpected lexer state " + to_string(state) };
            }
        }
    }


    [[nodiscard]] std::vector<Token> lex_until_linefeed(const std::string& s)
    {
        using _fsa   = FiniteStateAutomata;
        using _state = _fsa::State;
        //using _ttype = Token::Type;

        static const FiniteStateAutomata fsa{};

        // empty strings disallowed
        assert(!std::empty(s));

        std::vector<Token> tokens;
        for (auto pos = std::data(s);;)
        {
            auto state = fsa.start_state();
            // skip whitespace and comments
            for (; fsa.skip_state(state); ++pos)
                state = fsa.advance(state, static_cast<unsigned char>(*pos));

            // save the starting position of the token
            auto begin = pos - 1;
            for (; !fsa.final_state(state); ++pos)
                state = fsa.advance(state, static_cast<unsigned char>(*pos));


            if (_state::final_alphanum == state)
            {
                --pos; // rollback last character
                //tokens.push_back({ .first = begin, .last = pos - 1, .type = _type::alphanum });
                //tokens.emplace_back(_ttype::alphanum, begin, pos);
                tokens.emplace_back(begin, pos);
                continue;
            }

            if (_state::final_newline == state ||
                _state::final_input_end == state) // reached end of input
                break;

            if (_state::final_error == state)
                throw std::runtime_error{ "Unexpected character while in state " + to_string(state) };

            throw std::logic_error{ "Unexpected lexer state " + to_string(state) };
        }

        // all tokens must be non-empty strings
        assert(std::none_of(std::cbegin(tokens), std::cend(tokens),
            [](const auto& t) { return std::empty(t); }));

        return tokens;
    }

    [[nodiscard]] const char* lex_until_linefeed(const char* pos, std::vector<Token>& tokens)
    {
        using _state = FiniteStateAutomata::State;

        assert(nullptr != pos);
        assert('\0' != *pos);

        static const FiniteStateAutomata fsa{};
        for (;;)
        {
            auto state = fsa.start_state();
            // skip whitespace and comments
            for (; fsa.skip_state(state); ++pos)
                state = fsa.advance(state, static_cast<unsigned char>(*pos));

            // save the starting position of the token
            auto begin = pos - 1;
            for (; !fsa.final_state(state); ++pos)
                state = fsa.advance(state, static_cast<unsigned char>(*pos));

            if (_state::final_alphanum == state)
            {
                --pos;               // rollback last character
                assert(pos > begin); // at least one char in the token
                tokens.emplace_back(std::string_view{ begin, static_cast<std::size_t>(std::distance(begin, pos)) });
                continue;
            }

            if (_state::final_newline == state)
                break;

            if (_state::final_input_end == state) // reached end of input
            {
                --pos;
                break;
            }

            if (_state::final_error == state)
                throw std::runtime_error{ "Unexpected character while in state " + to_string(state) };

            throw std::logic_error{ "Unexpected lexer state " + to_string(state) };
        }

        assert('\0' == pos[0] || '\n' == pos[-1]);
        return pos;
    }

} // namespace obj