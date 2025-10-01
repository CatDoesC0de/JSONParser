/*
  SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */

#pragma once

#include "buffer.hpp"

enum class token_type
{
    token_end_of_stream,
    token_error,

    token_left_curly,
    token_right_curly,
    token_left_square,
    token_right_square,

    token_colon,
    token_comma,

    token_string,
    token_true,
    token_false,
    token_null,
    token_number
};

struct token 
{
    token_type Type;
    buffer Value;
};

struct lexer
{
    buffer Source;
    u64 Index;
    u64 Line;
    u64 LineBegin;
};

void Error(lexer& Lexer, const char* Stage, const char* Message);
token NextToken(lexer& Lexer);
