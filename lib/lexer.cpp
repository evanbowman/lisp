#include "lexer.hpp"
#include <iostream>

namespace lisp {
Lexer::Token Lexer::lex()
{
    if (position_ < input_.size()) {
    RETRY:
        switch (current()) {
        case '[':
        case '(':
            position_++;
            return Token::LPAREN;
        case ']':
        case ')':
            position_++;
            return Token::RPAREN;
        case ';':
            while (position_ < input_.size()) {
                position_++;
                if (current() == '\n' or current() == '\r') {
                    position_++;
                    goto RETRY;
                }
            }
        case ' ':
        case '\n':
        case '\r':
            position_++;
            goto RETRY;
        case '\'':
            position_++;
            return Token::QUOTE;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            inputBuffer_.clear();
            for (; checkTermCond(); ++position_) {
                if (not std::isdigit(current())) {
                    if (current() == '.') {
                        goto TOKENIZE_FLOAT;
                    } else {
                        goto TOKENIZE_SYMBOL;
                    }
                } else {
                    inputBuffer_.push_back(current());
                }
            }
            return Token::INTEGER;
        }
        case '\"': {
            inputBuffer_.clear();
            while (++position_ < input_.size()) {
                if (current() == '\"') {
                    ++position_;
                    return Token::STRING;
                } else {
                    inputBuffer_.push_back(current());
                }
            }
            return Token::NONE;
        }
        default:
            inputBuffer_.clear();
        TOKENIZE_SYMBOL:
            for (; checkTermCond(); ++position_) {
                inputBuffer_.push_back(current());
            }
            return Token::SYMBOL;
        }
    } else {
        return Token::NONE;
    }
TOKENIZE_FLOAT:
    for (; checkTermCond(); ++position_) {
        if (not std::isdigit(current())) {
            goto TOKENIZE_SYMBOL;
        } else {
            inputBuffer_.push_back(current());
        }
    }
    return Token::FLOAT;
}
} // namespace lisp