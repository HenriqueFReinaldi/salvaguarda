#include "linus.h"

int createdir(const char* path){
    return !mkdir(path, 0777);
}

int path_acessible(const char* path){
    return access(path, (F_OK | R_OK)) == 0;
}

int is_folder(const char* path){
    struct stat metadata;
    if (stat(path, &metadata) != 0){
        return 0;
    }
    return S_ISDIR(metadata.st_mode);
}

int copy_file(char* src, char* dst){
    
    int src_f = open(src, O_RDONLY);
    if (src_f < 0 ) {
        return 0;
    }

    int dst_f = open(dst, (O_WRONLY | O_CREAT | O_EXCL), (S_IRUSR | S_IWUSR));
    if (dst_f < 0){
        close(src_f);
        return 0;
    }

    struct stat src_metadata;
    if (fstat(src_f, &src_metadata) > 0) {
        close(src_f);
        close(dst_f);
        return 0;
    }

    off_t f = 0;
    if (sendfile(dst_f, src_f, &f, src_metadata.st_size) < 0){
        close(src_f);
        close(dst_f);
        return 0;
    }

    close(src_f);
    close(dst_f);
    return 1;
}

int get_program_path(char* path){
    ssize_t q = readlink("/proc/self/exe", path, MAX_PATH-1);
    if (q == -1) return 0;
    path[q] = '\0';
    return 1;
}

int get_working_path(char* path){
    if (getcwd(path, MAX_PATH) != NULL) return 1;
    return 0;
}





DirIt* init_dirit(char* path, int recursive){
    DirIt* p = (DirIt*)malloc(sizeof(DirIt));
    p->recursive = recursive;
    p->path = path;
    p->prev = NULL;
    p->dir = opendir(path);
    p->depth = 0;
    return p;
}

int file_travel(char* nome, int* is_file, DirIt* dit){
    struct dirent* entry;
    while (1) {
        if (dit->init_call) dit->init_call = 0;
        if ((entry = readdir(dit->dir)) == NULL) return 0;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        strncpy(nome, entry->d_name, MAX_PATH);
        *is_file = (entry->d_type == 8);
        return 1;
    }
}

int skip_folder(DirIt** dit){
    closedir((*dit)->dir);
    if ((*dit)->prev == NULL){
        return 0;
    }
    *dit = (*dit)->prev;
    return 2;
}

int path_travel(DirIt** dit){
    if ((*dit)->dir == NULL) return 0;

    char nome[MAX_PATH];
    int is_file;
    if (!file_travel(nome, &is_file, (*dit))) return skip_folder(dit);

    char* item_path = malloc(MAX_PATH);
    snprintf(item_path, MAX_PATH-1, "%s/%s", (*dit)->path, nome);

    if (!is_file) {
        if ((*dit)->recursive){
            DirIt* newdit = init_dirit(item_path, 1);
            newdit->prev = (*dit);
            newdit->depth = (*dit)->depth+1;

            (*dit) = newdit;
        }
    }

    (*dit)->item_name = strrchr(item_path, '/')+1;
    (*dit)->item_path = item_path;
    (*dit)->is_file = is_file;
 
    return 1;
}