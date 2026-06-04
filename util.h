#ifndef UTIL_H
#define UTIL_H

#define VER "04.06.2026.1"

#include "set/set.h"
#include "os/win.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdarg.h>



// #define COLORLESS

#ifdef COLORLESS
    #define COR_OK "\x1b[0m"
    #define COR_NOK "\x1b[0m"
    #define COR_ACC "\x1b[0m"
    #define ORANGE "\x1b[0m"
#else
    #define COR_OK "\033[97m"
    #define COR_NOK "\033[41m"
    #define COR_ACC "\x1b[35m"
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


int count_files(char* path, int recursion);
void display_dir(char* path, int recursion, int show_files, int show_folders);
//int fileTravel(char* path, int recursion, int show_files, int show_folders);


void intro();

#endif