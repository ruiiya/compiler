#pragma once

#include "grammar.h"

#include <string>
#include <fstream>
#include <sstream>

using namespace std;

namespace xd
{
    struct token {
        string token_type;
        string value;

        token() = default;

        token(string token_type, string value) : token_type(std::move(token_type)), value(std::move(value)) {};
    };

    class lexer {

        // https://stackoverflow.com/questions/447206/c-isfloat-function
        bool isFloat( string myString ) {
            std::istringstream iss(myString);
            float f;
            iss >> noskipws >> f; // noskipws considers leading whitespace invalid
            // Check the entire string was consumed and if either failbit or badbit is set
            return iss.eof() && !iss.fail(); 
        }

        string error;

        public:

        const string& get_error() {
            return error;
        }

        token get_token(ifstream& ifs,const vector<string>& terminals) {
            static char last_char = ' ';
            while (!ifs.eof() && isspace(last_char)) {
                ifs.get(last_char);
            }

            if(ifs.eof()) {
                return token("__EOF__", "__EOF__");
            }

            if(isalpha(last_char)) {
                string identifier_string;
                identifier_string += last_char;
                while(ifs.get(last_char) && isalnum(last_char)) {
                    identifier_string += last_char;
                }

                if (ifs.eof()) {
                    last_char = ' ';
                }

                if(find(terminals.begin(), terminals.end(), identifier_string) != terminals.end()) {
                    return token(identifier_string, identifier_string);
                } else if(identifier_string == "true" || identifier_string == "false") {
                    return token("boolean_constant", identifier_string);
                } else {
                    return token("identifier", identifier_string);
                }
            }

            if(isdigit(last_char) || last_char == '.') {
                string num_string;

                bool has_dot = false;
                bool has_e = false;
                bool has_sign = false;
                bool has_digits_before_dot = false;
                bool has_digits_after_dot = false;
                bool has_digits_after_e = false;

                bool is_error = false;

                do {
                    if (last_char == '.') {
                        if (has_dot || has_e) {
                            is_error = true;
                            break;
                        }
                        has_dot = true;
                    } else if (last_char == 'e' || last_char == 'E') {
                        if (has_e) {
                            is_error = true;
                            break;
                        }
                        has_e = true;
                    } else if (isdigit(last_char)) {
                        if (has_e) {
                            has_digits_after_e = true;
                        } else if (has_dot) {
                            has_digits_after_dot = true;
                        } else {
                            has_digits_before_dot = true;
                        }
                    } else if(isalpha(last_char)) {
                        is_error = true;
                        break;
                    } else {
                        break;
                    }
                    num_string += last_char;
                }while(!ifs.eof() && ifs.get(last_char));

                if(isalpha(last_char)) {
                    num_string += last_char;
                    while(ifs.get(last_char) && isalnum(last_char)) {
                        num_string += last_char;
                    }
                }

                if (ifs.eof()) {
                    last_char = ' ';
                }

                if(is_error) {
                    error = "Wrong fractional number format : " + num_string;
                    return token("__ERROR__", "__ERROR__");
                }

                for(char& c : num_string) {
                        if(!isdigit(c)) {
                            return token("float_constant", num_string);
                        }
                    }
                return token("integer_constant", num_string);
            }

            if(ispunct(last_char)) {
                token tok;
                static const vector<char> unary_punct({'(', ')', '[', ']', '{', '}', '*', '+', '-', '%', ',', ';'});
                if(find(unary_punct.begin(), unary_punct.end(), last_char) != unary_punct.end()) {

                    tok = token(string(1,last_char), string(1,last_char));
                    ifs.get(last_char);
                    if (ifs.eof()) {
                        last_char = ' ';
                    }
                } else {
                    string literal_string = "";
                    switch (last_char)
                    {
                    case '&':
                        ifs.get(last_char);
                        if(last_char == '&') {
                            ifs.get(last_char);
                            tok = token("&&", "&&");
                            break;
                        }
                        error = "Unexpected Token";
                        tok = token("__ERROR__", "__ERROR__");
                        break;
                    case '|':
                        ifs.get(last_char);
                        if(last_char == '|') {
                            ifs.get(last_char);
                            tok = token("||", "||");
                            break;
                        }
                        error = "Unexpected Token";
                        tok = token("__ERROR__", "__ERROR__");
                        break;
                    case '=':
                        ifs.get(last_char);
                        if(last_char == '=') {
                            ifs.get(last_char);
                            tok = token("==", "==");
                            break;
                        }
                        tok =  token("=", "=");
                        break;

                    case '!':
                        ifs.get(last_char);
                        if(last_char == '=') {
                            ifs.get(last_char);
                            tok =  token("!=", "!=");
                            break;
                        }
                        tok =  token("!", "!");
                        break;

                    case '>':
                        ifs.get(last_char);
                        if(last_char == '=') {
                            ifs.get(last_char);
                            tok =  token(">=", ">=");
                            break;
                        }
                        tok =  token(">", ">");
                        break;
                    
                    case '<':
                        ifs.get(last_char);
                        if(last_char == '=') {
                            ifs.get(last_char);
                            tok =  token("<=", "<=");
                            break;
                        }
                        tok =  token("<", "<");
                        break;

                    case '"':
                        do {
                            ifs.get(last_char);
                            literal_string += last_char;
                        }while(!ifs.eof() && last_char != '"');
                        if(ifs.eof()) {
                            error = "Wrong literal format";
                            tok = token("__ERROR__", "__ERROR__");
                            break;
                        }

                        literal_string.pop_back();
                        ifs.get(last_char);
                        tok = token("string_constant", '"' + literal_string + '"');
                        break;

                    case '/':
                        ifs.get(last_char);
                        if (last_char == '/') {
                            do {
                                ifs.get(last_char);
                            }while(!ifs.eof() && last_char != '\n' && last_char != '\r');
                            ifs.get(last_char);
                            tok = token("__COMMENT__", "__COMMENT__");
                            break;
                        } else {
                            tok = token("/", "/");
                            break;
                        }
                        break;
                    
                    default:
                        error = "Unexpected Token";
                        tok = token("__ERROR__", "__ERROR__");
                        break;
                    }
                    if (ifs.eof()) {
                        last_char = ' ';
                    }

                    
                }
                
                return tok;
            }
            error = "Unexpected Token";
            return token("__ERROR__", "__ERROR__");
        } 
    };
} // namespace xd
