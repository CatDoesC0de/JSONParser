/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */

#include <stdio.h>

#include "lexer.hpp"

u8 Consume(lexer& Lexer)
{
    return IndexInBounds(Lexer.Source, Lexer.Index) ? Lexer.Source.Data[Lexer.Index++] : '\0';
}

u8 Peek(lexer& Lexer)
{
    return IndexInBounds(Lexer.Source, Lexer.Index) ? Lexer.Source.Data[Lexer.Index] : '\0';
}

void Error(lexer& Lexer, const char* Stage, const char* Message)
{
    fprintf(stderr, "ERROR - [%s] @ (%lu,%lu): %s\n", 
    Stage, Lexer.Line, Lexer.Index - Lexer.LineBegin, Message);
}

void LexerError(lexer& Lexer, const char* Message)
{
    Error(Lexer, "Lexer", Message);
}

b32 IsJSONWhitespace(u8 Character)
{
    b32 Result =
        Character == '\n'
        || Character == '\t'
        || Character == '\r'
        || Character == ' ';

    return Result;
}

b32 ReadWhitespace(lexer& Lexer)
{

    u8 Character = Peek(Lexer);

    if (Character == '\n')
    {
        Lexer.Line++;

        if (IndexInBounds(Lexer.Source, Lexer.Index + 1))
        {
            Lexer.LineBegin = Lexer.Index + 1;
        }
    }

    if (IsJSONWhitespace(Character))
    {
        Consume(Lexer);
    }
    
    return IsJSONWhitespace(Peek(Lexer));
}

b32 IsJSONDigit(u8 Character)
{
    b32 Result = (Character >= '0') && (Character <= '9');
    return Result;
}

token ParseKeyword(buffer& Source, u64& Index, buffer ExpectedKeyword, token_type ExpectedTokenType)
{
    token ResultToken = {};
    ResultToken.Type = token_type::token_error;

    int i = 0;
    b32 Valid = true;
    for (;i < ExpectedKeyword.Length && IndexInBounds(Source, Index + i); i++)
    {
        if (ExpectedKeyword.Data[i] != Source.Data[Index + i])
        {
            Valid = false;
            break;
        }
    }

    if (Valid)
    {
        ResultToken.Type = ExpectedTokenType;
        ResultToken.Value.Data = Source.Data + Index - 1;
        ResultToken.Value.Length = ExpectedKeyword.Length + 1;
    }


    Index += i;
    return ResultToken;
}

token NextToken(lexer& Lexer)
{
    token ResultToken = {};
    ResultToken.Type = token_type::token_error;

    buffer& Source = Lexer.Source;
    u64& Index = Lexer.Index;

    if (IndexInBounds(Source, Index))
    {
        while (ReadWhitespace(Lexer)) {}
        
        ResultToken.Value.Data = Source.Data + Index;
        ResultToken.Value.Length = 1;
        
        u8 CharacterAt = Consume(Lexer);
        switch (CharacterAt)
        {
            case '{': {ResultToken.Type = token_type::token_left_curly; } break;
            case '}': {ResultToken.Type = token_type::token_right_curly; } break;
            case '[': {ResultToken.Type = token_type::token_left_square; } break;
            case ']': {ResultToken.Type = token_type::token_right_square; } break;
            case ':': {ResultToken.Type = token_type::token_colon; } break;
            case ',': {ResultToken.Type = token_type::token_comma; } break;
            case '"':
            {
                ResultToken.Type = token_type::token_string;
                ResultToken.Value.Data = Source.Data + Index;
                u64 StringStart = Index;
                
                while (IndexInBounds(Source, Index) 
                        && Peek(Lexer) != '"')
                {
                    if (IndexInBounds(Source, Index + 1) 
                            && Peek(Lexer) == '\\'
                            && Source.Data[Index + 1] == '"'
                        )
                    {
                        Consume(Lexer);
                    }

                    Consume(Lexer);
                }

                ResultToken.Value.Length = Index - StringStart;

                if (IndexInBounds(Source, Index + 1))
                {
                    Consume(Lexer);
                }

            }
            break;
            case 't': { ResultToken = ParseKeyword(Source, Index, FROM_CONSTANT_STRING("rue"), token_type::token_true);} break;
            case 'f': { ResultToken = ParseKeyword(Source, Index, FROM_CONSTANT_STRING("alse"), token_type::token_false);} break;
            case 'n': { ResultToken = ParseKeyword(Source, Index, FROM_CONSTANT_STRING("ull"), token_type::token_null);} break;

            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                u64 NumberStart = Index - 1;
                ResultToken.Type = token_type::token_number;

                if (CharacterAt == '-')
                {
                    CharacterAt = Consume(Lexer);
                }

                if (CharacterAt != '0')
                {
                    while (IsJSONDigit(Peek(Lexer))) 
                    {
                        Consume(Lexer);
                    }
                }
                else if (Peek(Lexer) != '.')
                {
                    LexerError(Lexer, "Expected decimal after 0.");
                    break;
                }

                if (Peek(Lexer) == '.')
                {
                    Consume(Lexer);

                    while (IsJSONDigit(Peek(Lexer))) 
                    {
                        Consume(Lexer);
                    }

                    u8 E = Peek(Lexer);

                    if (E == 'e' || E == 'E')
                    {
                        Consume(Lexer);
                    }

                    u8 Sign = Peek(Lexer);

                    if (Sign == '+' || Sign == '-')
                    {
                        Consume(Lexer);
                    }

                    while (IsJSONDigit(Peek(Lexer))) 
                    {
                        Consume(Lexer);
                    }
                }

                ResultToken.Value.Length = Index - NumberStart;
            }
            break;

            default:
            {
                LexerError(Lexer, "Unexpected character during tokenization.");
            }
            break;
        }
    }
    else 
    {
        ResultToken.Type = token_type::token_end_of_stream;
    }

    return ResultToken;
}
