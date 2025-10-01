/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */

#include <stdio.h>

#include "buffer.hpp"

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

b32 IndexInBounds(buffer& Buffer, u64 Index)
{
    return Index < Buffer.Length;
}
