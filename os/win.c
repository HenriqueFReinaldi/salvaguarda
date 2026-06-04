#include "win.h"

int createdir(const char* path){
    return CreateDirectory(path, NULL);
}

int path_acessible(const char* path){
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

int is_folder(const char* path){
    DWORD at = GetFileAttributesA(path);
    if (at == INVALID_FILE_ATTRIBUTES) return 0;
    return (at & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

int copy_file(char* src, char* dst){
   return CopyFile(src, dst, 0);
}

int get_program_path(char* path){
    return GetModuleFileNameA(NULL, path, MAX_PATH);
}

int get_working_path(char* path){
    return GetCurrentDirectoryA(MAX_PATH, path);
}


int file_travel(char* nome, int* is_file, DirIt* dit){
    while (1) {
        if (dit->init_call) dit->init_call = 0;
        else if (!FindNextFile(dit->hFind, &dit->data)) return 0;

        if (strcmp(dit->data.cFileName, ".") == 0 || strcmp(dit->data.cFileName, "..") == 0) continue;

        strncpy(nome, dit->data.cFileName, MAX_PATH);
        *is_file = (dit->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
        return 1;
    }
}

DirIt* init_dirit(char* path, int recursive){
    DirIt* p = (DirIt*)malloc(sizeof(DirIt));
    p->recursive = recursive;
    p->status = 0;
    p->path = path;
    p->prev = NULL;
    return p;
}

int skip_folder(DirIt** dit){
    if ((*dit)->hFind != INVALID_HANDLE_VALUE){
        FindClose((*dit)->hFind);
        (*dit)->hFind = INVALID_HANDLE_VALUE;
    }
    if ((*dit)->prev == NULL){
        return 0;
    }
    *dit = (*dit)->prev;
    return 2;
}

int path_travel(DirIt** dit){
    if (!(*dit)->status){
        char new_path[MAX_PATH];
        snprintf(new_path, MAX_PATH, "%s\\*", (*dit)->path);

        (*dit)->hFind = FindFirstFile(new_path, &(*dit)->data);
        (*dit)->init_call = 1;

        if ((*dit)->hFind == INVALID_HANDLE_VALUE){
            return 0;
        }
        (*dit)->status = 1;
    }


    char nome[MAX_PATH];
    int is_file;
    int result = file_travel(nome, &is_file, (*dit));

    if (!result){
        return skip_folder(dit);
    }

    char* item_path = malloc(MAX_PATH);
    snprintf(item_path, MAX_PATH, "%s\\%s", (*dit)->path, nome);

    if (!is_file) {
        if ((*dit)->recursive){
            DirIt* newdit = (DirIt*)malloc(sizeof(DirIt));
            newdit->recursive = 1;
            newdit->status = 0;
            newdit->path = item_path;
            newdit->prev = (*dit);
            newdit->depth = (*dit)->depth+1;

            (*dit) = newdit;
        }
    }

    (*dit)->item_name = strrchr(item_path, '\\')+1;
    (*dit)->item_path = item_path;
    (*dit)->is_file = is_file;
 
    return 1;
}