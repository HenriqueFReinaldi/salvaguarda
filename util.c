#include "util.h"


int endsWith(char* a, char* b){
    size_t la = strlen(a);
    size_t lb = strlen(b);

    if (lb > la) return 0;

    return memcmp(a+(la-lb), b, lb) == 0;
}

int startsWith(char* a, char* b){
    size_t la = strlen(a);
    size_t lb = strlen(b);

    if (lb > la) return 0;

    return memcmp(a, b, lb) == 0;
}

int readFile(FILE* f, size_t* size, char** content){
    if (!f || !size || !content) return -1;

    if (fseek(f, 0, SEEK_END) != 0) return -1;
    long file_size = ftell(f);
    if (file_size < 0) return -1;

    *size = (size_t)file_size;
    if (fseek(f, 0, SEEK_SET) != 0) return -1;

    *content = malloc(*size + 1);
    if (!*content) return -1;

    size_t nread = fread(*content, 1, *size, f);
    if (nread != *size){ free(*content); return -1; }

    (*content)[*size] = '\0';
    return 0;
}

int noMallocReadFile(FILE* f, size_t size, char* content){
    if (!f || !size || !content) return -1;
    if (fseek(f, 0, SEEK_SET) != 0) return -1;

    size_t nread = fread(content, 1, size-1, f);
    content[nread] = '\0';
    return 0;
}

int staticGrowReadFile(FILE* f, size_t* size, char** content, size_t* content_size){
    if (!f || !size || !content) return -1;

    if (fseek(f, 0, SEEK_END) != 0) return -2;
    long file_size = ftell(f);
    if (file_size < 0) return -3;

    if ((size_t)file_size > (*size)){
        char* temp = realloc(*content, (size_t)file_size + 1);
        if (!temp) return -4;

        (*size) = (size_t)file_size;
        (*content) = temp;
    }

    if (fseek(f, 0, SEEK_SET) != 0) return -5;
    if (!*content) return -6;

    size_t nread = fread(*content, 1, file_size, f);
    if (nread > (*size))return -7;

    (*content)[nread] = 0;
    (*content_size) = nread;
    return 0;
}

int writeFile(FILE* f, char* content){
    if (!f) return -1;

    if(fprintf(f, "%s", content) < 0){
        return -1;
    }

    return 0;
}

int getFileLines(FILE* f, char*** lines, size_t* qtd_linhas){//limite de 300 linhas!!!
    size_t size;
    char* content;
    readFile(f, &size, &content);
    size_t lc = 1;

    size_t line_size[300] = {0};
    size_t ls_index = 0;

    for (size_t i = 0; i < size; i++){
        line_size[ls_index]++;
        if(content[i] == '\n') {
            lc++;
            ls_index++;
        }
    }

    (*lines) = malloc(lc*sizeof(char*));

    size_t last_index = 0;
    for (size_t i = 0; i <= ls_index; i++){
        size_t offset = 0;
        size_t a = last_index+line_size[i];

        if (a-1 >= 0 && content[a-1] == '\n') offset++;
        if (a-2 >= 0 && content[a-2] == '\r') offset++;

        char* new_line;
        if (offset < line_size[i]){
            new_line = malloc(line_size[i]-offset+1);
            memcpy(new_line, content+last_index, line_size[i]-offset);
            new_line[line_size[i]-offset] = '\0';
        }
        else new_line = "";
        
        
        last_index += line_size[i];
        (*lines)[i] = new_line;
    }
    
    (*qtd_linhas) = ++ls_index;

    return 0;
}

void msgExit(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(0);
}

int fileTravel(char* path, int recursion, int show_files, int show_folders){
    int count = 0;

    WIN32_FIND_DATA fd;
    char new_path[MAX_PATH];
    snprintf(new_path, MAX_PATH, "%s\\*", path);


    HANDLE hFind = FindFirstFile(new_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    do {
        if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")){
            
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                if (show_folders) {
                    for (int i = 1; i < recursion; i++) printf("    ");
                    printf(BLUE"/%s\n" RESET, fd.cFileName);
                }

                if (recursion > 0){
                    char folder_path[MAX_PATH];
                    snprintf(folder_path, MAX_PATH, "%s\\%s", path, fd.cFileName);
                    count += fileTravel(folder_path, recursion+1, show_files, show_folders);
                }
            }
            else{
                if (show_files){
                    for (int i = 1; i < recursion; i++) printf("    ");
                    printf("%s\n", fd.cFileName);
                } 
                count++;
            }
        }
    } while(FindNextFile(hFind, &fd));

    FindClose(hFind);
    return count;;
}

void intro(){
    printf("salt {registro}                 | Cria uma nova build e manda pro registro.\n");
    printf("salt -new {registro}            | Cria um novo registro.\n");
    printf("salt -load{n} {registro}        | Carrega a última versão (ou uma versão {n}) do registro pro diretório atual.\n");
    printf("salt -from{d} {registro}        | Troca o diretório de execução para {d}.\n\n");

    printf("salt -view{build} {registro}    | Mostra todas as builds (ou uma determinada) do registro.\n");
    printf("salt -info {registro}           | Mostra mensagens de copiar.\n");
    printf("salt -esp{nome} {registro}      | Cria uma build com nome especial.\n");
    printf("salt -msg{mensagem} {registro}  | Manda uma mensagem para as logs.\n");

    printf("Nota: não inclua as chaves nos comandos.\n\n");

    printf("Inclua o comando 'tignore {.tipo}' no arquivo svconfig.txt para excluir tipos de arquivos das builds.\n");
    printf("Inclua o comando 'dignore {dir}' no arquivo svconfig.txt para excluir certos diretorios das builds;");
}