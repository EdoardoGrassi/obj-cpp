#pragma once
#ifndef OBJCPP_LEXER_HPP
#define OBJCPP_LEXER_HPP

#include <array>
#include <cassert>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace obj
{
    /*
    /// @brief Token produced by the lexer.
    struct Token
    {
        enum class Type
        {
            alphanum,
            endline,
        };

        Token(Type t, const char* first, const char* last) noexcept
            : _first{ first }, _last{ last }, _type{ t }
        {
            assert(first < last); // at least one character
        }

        Token(Type t, const std::string_view s) noexcept
            : _first{ std::data(s) }, _last{ std::data(s) + std::size(s) }, _type{ t }
        {
            assert(!std::empty(s)); // at least one character
        }

        [[nodiscard]] const char* first() const noexcept { return _first; }
        [[nodiscard]] const char* last() const noexcept { return _last; }
        [[nodiscard]] Type        type() const noexcept { return _type; }

        [[nodiscard]] std::string_view view() const noexcept
        {
            return std::string_view{ _first, _last };
        }

    private:
        const char* _first;
        const char* _last;
        Type        _type;
    };

    [[nodiscard]] inline bool operator==(const Token& lhs, const Token& rhs) noexcept
    {
        return lhs.type() == rhs.type() && lhs.view() == rhs.view();
    }

    [[nodiscard]] inline bool operator!=(const Token& lhs, const Token& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] inline std::string to_string(const Token::Type& type)
    {
        using TT = Token::Type;
        switch (type)
        {
            // TODO: replace strings with STRINGIZE macro
            case TT::alphanum: return "Token::Type::alphanum";
            case TT::endline: return "Token::Type::endline";
            default: return "unknown";
        }
    }

    inline std::ostream& operator<<(std::ostream& os, const Token& t)
    {
        return os << "view: " << t.view() << ", type: " << to_string(t.type());
    }
    */

    using Token = std::string_view;

    class FiniteStateAutomata
    {
    public:
        enum class State : unsigned char
        {
            whitespace  = 0,          // whitespace
            start_state = whitespace, // initial state of the automata
            alphanum,                 // sequence of non-whitespace characters
            comment,                  // comment text

            last_nonfinal_state = comment,
            //^^^ non-final states ^^^/vvv final states vvv
            first_final_state,

            final_alphanum,  // identifier
            final_newline,   // locale-indipenendent end of line
            final_input_end, // reached the end of the input sequence
            final_error,     // error states sink

            last_state = final_error
        };

        enum class EquivClass : unsigned char
        {
            alphanum = 0, // alphanumeric strings ([a..z][A..Z][0..9])
            whitespace,   // ignored whitespace (spaces)
            comment,      // comment start (#)
            lf,           // line feed (Unix line terminator \n)
            st,           // c/c++ string terminator (\0)
            invalid,      // invalid characters (non-ascii characters)

            last_valid_value = invalid
        };

        [[nodiscard]] constexpr FiniteStateAutomata() noexcept
        {
            using _s  = State;
            using _ec = EquivClass;

            const unsigned char INPUT_END      = '\0';
            const unsigned char COMMENT_BEG    = '#';
            const unsigned char LINEFEED       = '\n';
            const unsigned char WHITESPACE     = ' ';
            const unsigned char IDENTIFIER_BEG = '!'; // first printable ASCII character
            const unsigned char IDENTIFIER_END = '~'; // last printable ASCII character

            // fill with invalid class then overwrite with other classes
            std::fill(std::begin(_eq_classes), std::end(_eq_classes), _ec::invalid);
            for (auto c = IDENTIFIER_BEG; c != IDENTIFIER_END; ++c)
                _eq_classes[c] = _ec::alphanum;

            _eq_classes[INPUT_END]   = _ec::st;
            _eq_classes[WHITESPACE]  = _ec::whitespace;
            _eq_classes[COMMENT_BEG] = _ec::comment;
            _eq_classes[LINEFEED]    = _ec::lf;

            // fill automata transition table
            std::fill_n(&_fsa[0][0], sizeof(_fsa) / sizeof(decltype(_fsa[0][0])), _s::final_error);

            t(_s::whitespace, _ec::alphanum)   = _s::alphanum;
            t(_s::whitespace, _ec::comment)    = _s::comment;
            t(_s::whitespace, _ec::whitespace) = _s::whitespace;
            t(_s::whitespace, _ec::lf)         = _s::final_newline;
            t(_s::whitespace, _ec::st)         = _s::final_input_end;

            t(_s::comment, _ec::alphanum)   = _s::comment;
            t(_s::comment, _ec::comment)    = _s::comment;
            t(_s::comment, _ec::whitespace) = _s::comment;
            t(_s::comment, _ec::lf)         = _s::final_newline;
            t(_s::comment, _ec::st)         = _s::final_input_end;

            t(_s::alphanum, _ec::alphanum)   = _s::alphanum;
            t(_s::alphanum, _ec::comment)    = _s::final_alphanum;
            t(_s::alphanum, _ec::whitespace) = _s::final_alphanum;
            t(_s::alphanum, _ec::lf)         = _s::final_alphanum;
            t(_s::alphanum, _ec::st)         = _s::final_alphanum;
        }

        [[nodiscard]] constexpr State transition(State s, EquivClass c) const noexcept
        {
            return _fsa[static_cast<std::size_t>(s)][static_cast<std::size_t>(c)];
        }

        [[nodiscard]] constexpr State start_state() const noexcept
        {
            return State::start_state;
        }

        // Automata transition function.
        [[nodiscard]] constexpr State advance(State current, unsigned char c) const noexcept
        {
            return transition(current, _eq_classes[c]);
        }

        // Check whether a state can be skipped.
        [[nodiscard]] bool skip_state(State s) const noexcept
        {
            return s == State::whitespace || s == State::comment;
        }

        // Check whether a state is final.
        [[nodiscard]] bool final_state(State s) const noexcept
        {
            return s >= State::first_final_state;
        }

    private:
        //std::array<EquivClass, 256> _eq_classes;
        static constexpr auto nsymbols = 256; // unsigned char
        static constexpr auto nstates  = static_cast<std::size_t>(State::last_nonfinal_state);
        static constexpr auto nclasses = static_cast<std::size_t>(EquivClass::last_valid_value);

        EquivClass _eq_classes[nsymbols];
        State      _fsa[nstates][nclasses];

        [[nodiscard]] constexpr State& t(State s, EquivClass c) noexcept
        {
            return _fsa[static_cast<std::size_t>(s)][static_cast<std::size_t>(c)];
        }
    };

    [[nodiscard]] inline std::string to_string(const FiniteStateAutomata::State& s)
    {
        using FSA = FiniteStateAutomata;
        using LS  = FSA::State;
        switch (s)
        {
            case LS::alphanum: return "FSA::State::alphanum";
            case LS::whitespace: return "FSA::State::whitespace";
            case LS::comment: return "FSA::State::comment";
            case LS::final_alphanum: return "FSA::State::final_alphanum ";
            case LS::final_newline: return "FSA::State::final_newline";
            case LS::final_input_end: return "FSA::State::final_input_end ";
            case LS::final_error: return "FSA::State::final_error";
            default: return "unknown";
        }
    }

    /// @brief Parse tokens from the whole source text.
    [[nodiscard]] std::vector<Token> lex(std::span<const char> s);

    /// @brief Parse tokens from a single line of source text.
    ///
    /// @param[in]  from   Starting position for the lexing phase.
    /// @param[out] tokens Destination for produced tokens.
    ///
    /// @return Ending position of the lexing phase.
    [[nodiscard]] const char* lex_until_linefeed(const char* from, std::vector<Token>& tokens);

} // namespace obj

#endif // !OBJCPP_LEXER_HPP