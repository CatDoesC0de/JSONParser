/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Connor Andrew Taylor
 * Part of catdoescode-computer-enhance
 */

#include <stdio.h>
#include <sys/stat.h>

#include "json_parser.hpp"

buffer ReadFileContents(char* FileName)
{
    buffer Contents = {};
    FILE* File = fopen(FileName, "r");

    if (File)
    {
        struct stat Stat;
        stat(FileName, &Stat);

        Contents = AllocateBuffer(Stat.st_size);

        if (fread(Contents.Data, Stat.st_size, 1, File) != 1)
        {
            fprintf(stderr, "ERROR: Unable to read file %s.\n", FileName);
            FreeBuffer(Contents);
        }
        else 
        {
            Contents.Length = Stat.st_size;
        }
    }
    else 
    {
        fprintf(stderr, "ERROR: Unable to open file %s.\n", FileName);
    }

    return Contents;
}

int main(int Argc, char** Argv)
{
    if (Argc < 2)
    {
        printf("Usage: %s [pairs_file]\n", Argv[0]);
        return -1;
    }

    buffer InputJSON = ReadFileContents(Argv[1]);

    if (!InputJSON.Data)
    {
        return -1;
    }

    json_element* JSON = ParseJSON(InputJSON);
}
