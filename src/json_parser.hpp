/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */

#include <cstdint>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

typedef double f64;

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint32_t b32;

struct buffer 
{
    u8* Data;
    size_t Length;
};

#define FROM_CONSTANT_STRING(String) {(u8*)String, sizeof(String) - 1}

buffer AllocateBuffer(size_t Size)
{
    buffer Result;
    Result.Data = (u8*) malloc(Size);

    if (Result.Data)
    {
        Result.Length = Size;
    }
    else 
    {
        fprintf(stderr, "ERROR: Unable to allocate %lu bytes.\n", Size);
    }

    return Result;
}

void FreeBuffer(buffer& Buffer)
{
    if (Buffer.Data)
    {
        free(Buffer.Data);
    }

    Buffer = {};
}

b32 Equals(buffer& b1, buffer& b2)
{
    if (b1.Length != b2.Length)
    {
        return false;
    }

    b32 Result = true;
    if (b1.Data && b2.Data)
    {
        for (int i = 0; i < b1.Length; i++)
        {
            if (b1.Data[i] != b2.Data[i])
            {
                Result = false;
                break;
            }
        }
    }
    
    return Result;
}

b32 IndexInBounds(buffer& Buffer, u64 Index)
{
    return Index < Buffer.Length;
}

struct json_element
{
    buffer Key;
    buffer Value;    
    json_element* Child;
    json_element* Sibling;
};

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
                LexerError(Lexer, "Invalid token.");
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


void ParserError(lexer& Lexer, const char* Message)
{
    Error(Lexer, "Parser", Message);
}

json_element* ParseJSONElement(lexer& Lexer, buffer Key);
json_element* ParseJSONObject(lexer& Lexer, token_type ClosingToken, b32 HasKeys) 
{
    json_element* Head = {};
    json_element* Tail = {};

    while (true)
    {
        buffer Key = {};
        if (HasKeys)
        {
            token KeyToken = NextToken(Lexer);
            if (KeyToken.Type == token_type::token_string)
            {
                token Colon = NextToken(Lexer);

                if (Colon.Type == token_type::token_colon)
                {
                    Key = KeyToken.Value;
                }
                else 
                {
                    ParserError(Lexer, "Expected colon after key.");
                    break;
                }
            }
            else
            {
                ParserError(Lexer, "Object element must specify key");
                break;
            }
        }

        json_element* Element = ParseJSONElement(Lexer, Key);
        
        if (Element)
        {
            if (!Head)
            {
                Head = Tail = Element;
            }
            else 
            {
                Tail = Tail->Sibling = Element;
            }
        }
        else 
        {
            break;
        }

        token Comma = NextToken(Lexer);

        if (Comma.Type == token_type::token_comma)
        {
            continue;
        }
        else if (Comma.Type == ClosingToken)
        {
            break;
        }
        else 
        {
            ParserError(Lexer, "Unexpected token.");
            break;
        }
        
    }
    return Head;
}

json_element* ParseJSONElement(lexer& Lexer, buffer Key)
{
    token Token = NextToken(Lexer);

    json_element* ResultElement = {};

    if (
        Token.Type != token_type::token_error
        && Token.Type != token_type::token_end_of_stream
        )
    {

        b32 Valid = true;
        json_element* ChildElement = {};
        if (Token.Type == token_type::token_left_curly)
        {
            ChildElement = ParseJSONObject(Lexer, token_type::token_right_curly, true);        
        }
        else if (Token.Type == token_type::token_left_square)
        {
            ChildElement = ParseJSONObject(Lexer, token_type::token_right_square, false);        
        }
        else if (
                    Token.Type == token_type::token_false
                    || Token.Type == token_type::token_true
                    || Token.Type == token_type::token_null
                    || Token.Type == token_type::token_string
                    || Token.Type == token_type::token_number
                )
        {}
        else 
        {
            Valid = false;
            ParserError(Lexer, "Unexpected token.");
        }

        if (Valid)
        {
            ResultElement = (json_element*) malloc(sizeof(json_element));
            ResultElement->Key = Key;
            ResultElement->Child = ChildElement;
            ResultElement->Value = Token.Value;
        }
    }

    return ResultElement;
}

json_element* ParseJSON(buffer InputJSON)
{
    lexer Lexer = {};
    Lexer.Source = InputJSON;
    Lexer.Line = 1;

    json_element* ResultElement = {};
    ResultElement = ParseJSONElement(Lexer, {});
    return ResultElement;
}

b32 LexerConvertInteger(lexer& Lexer, f64& Result)
{
    f64 Converted = 0;
    b32 CouldConvert = true;

    if (IsJSONDigit(Peek(Lexer)))
    {
        u64 IntegerStart = Lexer.Index;
        f64 Base10IntegerPlace = 0;

        while (IsJSONDigit(Peek(Lexer)))
        {
            Base10IntegerPlace++;
            Consume(Lexer);
        }

        Lexer.Index = IntegerStart;
        
        while (IsJSONDigit(Peek(Lexer)))
        {
            u8 Digit = Consume(Lexer);
            f64 DigitConversion = (Digit - '0') * Base10IntegerPlace;
            Converted += DigitConversion;
            Base10IntegerPlace *= 0.1;
        }
    }
    else 
    {
        CouldConvert = false;
    }

    Result = Converted;
    return CouldConvert;
}

b32 ElementToF64(json_element* Element, f64& Result)
{
    b32 CouldConvert = true;

    f64 Converted = 0;
    f64 Sign = 1;

    lexer Lexer = {};
    Lexer.Index = 0;
    Lexer.Source = Element->Value;

    if (!IndexInBounds(Lexer.Source, Lexer.Index))
    {
        return false;
    }

    if (Peek(Lexer) == '-')
    {
        Sign = -1;
        Consume(Lexer); // -
    }
        
    if (!(IsJSONDigit(Peek(Lexer)) && LexerConvertInteger(Lexer, Converted)))
    {
        return false;
    }

    if (Peek(Lexer) == '.')
    {
        Consume(Lexer); // .
        
        f64 Fraction = 0;
        f64 Place = 0.1;
        while (IsJSONDigit(Peek(Lexer)))
        {
            u8 Digit = Consume(Lexer);
            f64 DigitConversion = (Digit - '0') * Place;

            Fraction += DigitConversion;
            Place *= 0.1;
        }
        Converted += Fraction;
    }

    u8 ESymbol = Peek(Lexer);
    if (ESymbol == 'e' || ESymbol == 'E')
    {
        Consume(Lexer); //E or e

        u8 SignSymbol = Peek(Lexer);
        f64 ExponentSign = 1;

        if (SignSymbol == '+' || SignSymbol == '-')
        {
            ExponentSign = SignSymbol == '-' ? -1 : 1;
            Consume(Lexer); // + or -
        }

        f64 Exponent = 0;

        if (!LexerConvertInteger(Lexer, Exponent))
        {
            return false;
        }

        Converted *= pow(10, Exponent * ExponentSign);
    }

    Converted *= Sign;
    Result = Converted;

    return true;
}

json_element* SearchKey(json_element* Object, const char* Key)
{
    json_element* Result;

    if (Object)   
    {
        buffer KeyLookup = {. Data = (u8*) Key, .Length = strlen(Key)};
        for (json_element* Child = Object->Child; Child; Child = Child->Sibling)
        {
            if (Equals(KeyLookup, Child->Key))
            {
                Result = Child;
                break;
            }
        }
    }

    return Result;
}

void PrintElementValue(json_element* Element)
{
    printf("%.*s", (u32) Element->Value.Length, (char*) Element->Value.Data);
}
