#ifndef UTIL_H
#define UTIL_H

#define VER "07.04.2026.1"

#include "set/set.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdarg.h>


#define COLORLESS
    #ifdef COLORLESS
        #define BLUE "\x1b[0m"
        #define RED "\x1b[0m"
        #define PURPLE "\x1b[0m"
        #define ORANGE "\x1b[0m"
    #else
        #define BLUE "\x1b[34m"
        #define RED "\033[31m"
        #define PURPLE "\x1b[35m"
        #define ORANGE "\x1b[38;5;208m"
    #endif

#define RESET "\x1b[0m"

int endsWith(char* a, char* b);
int startsWith(char* a, char* b);

int readFile(FILE* f, size_t* size, char** content);
int noMallocReadFile(FILE* f, size_t size, char* content);
int staticGrowReadFile(FILE* f, size_t* size, char** content, size_t* content_size);

int writeFile(FILE* f, char* content);

int getFileLines(FILE* f, char*** lines, size_t* qtd_linhas);

void msgExit(const char *fmt, ...);

int fileTravel(char* path, int recursion, int show_files, int show_folders);


void intro();

#endif