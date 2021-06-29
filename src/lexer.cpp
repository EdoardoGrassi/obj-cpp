#include "lexer.hpp"

#include <span>
#include <vector>

namespace obj
{
    void normalize_line_endings(std::span<char> s) noexcept
    {
        assert(s.back() == '\0');
        // convert Windows-style \r\n line endings to Unix-style \n
        for (std::size_t i = 0; i < std::size(s) - 1; ++i)
            if (s[i] == '\r' && s[i + 1] == '\n')
                s[i] = ' ';
    }

    [[nodiscard]] std::vector<Token> lex(std::span<char> s)
    {
        using _type  = Token::Type;
        using _state = FiniteStateAutomata::State;
        const static FiniteStateAutomata fsa{}; // TODO: create lexer objects on the fly / make users do it?
        //print_table(fsa);
        normalize_line_endings(s);

        assert(s.back() == '\0'); // input buffer end mark

        std::vector<Token> tokens;
        tokens.reserve(1024);
        for (auto pos = std::data(s);;)
        {
            auto state = fsa.start_state();
            // skip whitespace and comments
            while (fsa.skip_state(state))
            {
                const auto ch = static_cast<unsigned char>(*pos++);
                const auto ec = fsa.equivalence_class(ch);
                state         = fsa.transition(state, ec);
            }

            auto begin = pos - 1;
            while (!fsa.final_state(state))
            {
                assert(!fsa.skip_state(state));
                const auto ch = static_cast<unsigned char>(*pos++);
                const auto ec = fsa.equivalence_class(ch);
                state         = fsa.transition(state, ec);
            }
            //--pos; // backtrack last character

            switch (state)
            {
                case _state::final_alphanum:
                    --pos; // rollback last character
                    //tokens.push_back({ .first = begin, .last = pos - 1, .type = _type::alphanum });
                    tokens.emplace_back(_type::alphanum, begin, pos);
                    break;

                case _state::final_newline:
                    //tokens.push_back({ .first = begin, .last = pos, .type = _type::endline });
                    tokens.emplace_back(_type::endline, begin, pos);
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
} // namespace obj