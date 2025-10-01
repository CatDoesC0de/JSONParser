/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */

#include "lexer.hpp"
#include "json_parser.hpp"

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

        token EndToken = NextToken(Lexer);

        if  (EndToken.Type == ClosingToken)
        {
            break;
        }

        if (EndToken.Type != token_type::token_comma)
        {
            ParserError(Lexer, "Reached unexpected end of object.");
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
            ParserError(Lexer, "Unexpected token when parsing JSON.");
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
