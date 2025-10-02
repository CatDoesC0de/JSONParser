/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */
 
#pragma once

#define FROM_CONSTANT_STRING(String) {(u8*)String, sizeof(String) - 1}

#include <stdint.h>
#include <cstddef>

typedef uint8_t u8;
typedef uint64_t u64;
typedef uint32_t b32;

struct buffer 
{
    u8* Data;
    size_t Length;
};

buffer AllocateBuffer(size_t Size);
void FreeBuffer(buffer& Buffer);
b32 IndexInBounds(buffer& Buffer, u64 Index);
