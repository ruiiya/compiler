#pragma once

#include <string>

using namespace std;

namespace xd
{
    struct token {
        string token_type;
        string value;

        token(string token_type, string value) : token_type(std::move(token_type)), value(std::move(value)) {};
    };
} // namespace xd
