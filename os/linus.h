#ifndef LINUS
#define LINUS

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


#include <stdio.h>
#define PROG_OS "linux"
#define MAX_PATH 4096

#define PATH_SEP '/'
#define PATH_SEP_STR "/"

int createdir(const char* path);
int path_acessible(const char* path);
int is_folder(const char* path);
int copy_file(char* src, char* dst);

int get_program_path(char* path);
int get_working_path(char* path);

typedef struct DirIt {
    struct DirIt* prev;
    int status;
    char* path;

    DIR* dir;
    int init_call;
    int recursive;

    char* item_path;
    char* item_name;
    int is_file;
    int depth;
} DirIt;

DirIt* init_dirit(char* path, int recursive);
int skip_folder(DirIt** dit);
int path_travel(DirIt** dit);

#endif