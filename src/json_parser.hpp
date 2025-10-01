/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */


#include "buffer.hpp"

struct json_element
{
    buffer Key;
    buffer Value;    
    json_element* Child;
    json_element* Sibling;
};

json_element* ParseJSON(buffer InputJSON);
