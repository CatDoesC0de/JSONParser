/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */
 
#pragma once

#include "cat_common.hpp"
#define FROM_CONSTANT_STRING(String) {(u8*)String, sizeof(String) - 1}

struct buffer 
{
    u8* Data;
    size_t Length;
};

buffer AllocateBuffer(size_t Size);
void FreeBuffer(buffer& Buffer);
b32 IndexInBounds(buffer& Buffer, u64 Index);
