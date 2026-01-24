#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <openssl/sha.h>
#include "readwrite.h"

#define DEST "D:\\sv"

static int ignore_exes = 1;
static char dotexe[] = ".exe"; 

#define M_NEW 0x01
#define M_LOAD 0x02
#define M_EXE 0x04

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

int getFileHash(FILE* f, char hash[41]){
    size_t size;
    char* content;
    readFile(f, &size, &content);

    unsigned char out[SHA_DIGEST_LENGTH]; 
    SHA1((unsigned char*)content, size, out);

    for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(hash+i*2, "%02x", out[i]);
    }
    free(content);
    return 0;
}

void dirCopy(char* orig, char* dest){
    WIN32_FIND_DATA fd;
    char fontePath[MAX_PATH];
    char destPath[MAX_PATH];
    CreateDirectory(dest, NULL);

    sprintf(fontePath, "%s\\*", orig);
    HANDLE hFind = FindFirstFile(fontePath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")){
            sprintf(fontePath, "%s\\%s", orig, fd.cFileName);
            sprintf(destPath, "%s\\%s", dest, fd.cFileName);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                dirCopy(fontePath, destPath);
            } 
            else{
                CopyFile(fontePath, destPath, FALSE);
            }
        }
    } while(FindNextFile(hFind, &fd));

    FindClose(hFind);
}


int createCheckDir(char* dest){
    if (!(CreateDirectory(dest, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)){
        return -1;
    } 
    return 0;
}

void criarDirRegistro(char* path){
    if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
        printf("Reg ja existe.");
        return;
    }

    if(!CreateDirectory(path, NULL)){
        printf("erro");
        return;
    }
    char contentPath[MAX_PATH];
    char buildPath[MAX_PATH];
    sprintf(contentPath, "%s\\conteudo", path);
    sprintf(buildPath, "%s\\build", path);

    if(!CreateDirectory(contentPath, NULL)){
        printf("erro");
        return;
    }
    if(!CreateDirectory(buildPath, NULL)){
        printf("erro");
        return;
    }

    sprintf(buildPath, "%s\\build\\salver", path);

    FILE* f = fopen(buildPath, "w");
    if (!f){
        printf("erro");
        return;
    }
    fprintf(f, "0");
    fclose(f);


    printf("Criado novo registro: %s", path);
    return;
}


void hashCopyBuild(char* orig, char* dest, char* conteudo){
    WIN32_FIND_DATA fd;
    char fonte_path[MAX_PATH];
    char dest_path[MAX_PATH];
    CreateDirectory(dest, NULL);

    sprintf(fonte_path, "%s\\*", orig);
    HANDLE hFind = FindFirstFile(fonte_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")){
            sprintf(fonte_path, "%s\\%s", orig, fd.cFileName);
            sprintf(dest_path, "%s\\%s", dest, fd.cFileName);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                hashCopyBuild(fonte_path, dest_path, conteudo);
            }
            else if (!(ignore_exes && endsWith(fd.cFileName, dotexe))){
                FILE* f = fopen(fonte_path, "rb");
                char hash[41];
                getFileHash(f, hash);
                fclose(f);

                char file_path[MAX_PATH];
                sprintf(file_path, "%s\\%s",conteudo, hash);

                DWORD at = GetFileAttributesA(file_path);
                if(!(at != INVALID_FILE_ATTRIBUTES && !(at & FILE_ATTRIBUTE_DIRECTORY))){

                    CopyFile(fonte_path, file_path, FALSE);
                    printf("\ncopiado: %s -> %s", fonte_path, file_path);
                }

                f = fopen(dest_path, "wb");
                writeFile(f, hash);
                fclose(f);
            }
        }
    } while(FindNextFile(hFind, &fd));

    FindClose(hFind);
}

int newBuild(char* orig, char* dest){
    char buildPath[MAX_PATH];
    char conteudoPath[MAX_PATH];
    char salverPath[MAX_PATH];
    char newVerPath[MAX_PATH];
    sprintf(buildPath, "%s\\build", dest);
    sprintf(conteudoPath, "%s\\conteudo", dest);
    sprintf(salverPath, "%s\\salver", buildPath);

    char* content;
    size_t sz = 0;

    FILE* salf = fopen(salverPath, "rb");
    readFile(salf, &sz, &content);
    fclose(salf);
    int current_ver = atoi(content);

    sprintf(newVerPath, "%s\\%d", buildPath, current_ver+1);
    if(createCheckDir(newVerPath) != 0){
        printf("deu erro salvando.");
        return -1;
    }

    printf("%s",newVerPath);

    char new_ver[30];
    sprintf(new_ver, "%d", current_ver+1);

    salf = fopen(salverPath, "wb");
    writeFile(salf, new_ver);
    fclose(salf);

    hashCopyBuild(orig, newVerPath, conteudoPath);

    return 0;
}


void hashLoadBuild(char* orig, char* dest, char* conteudo){
    WIN32_FIND_DATA fd;
    char fonte_path[MAX_PATH];
    char dest_path[MAX_PATH];
    CreateDirectory(dest, NULL);

    sprintf(fonte_path, "%s\\*", orig);
    HANDLE hFind = FindFirstFile(fonte_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")){
            sprintf(fonte_path, "%s\\%s", orig, fd.cFileName);
            sprintf(dest_path, "%s\\%s", dest, fd.cFileName);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                hashLoadBuild(fonte_path, dest_path, conteudo);
            } 
            else{
                char* content_hold;
                size_t size = 0;
                FILE* f = fopen(fonte_path, "rb");
                readFile(f, &size, &content_hold);
                fclose(f);

                char file_content_path[MAX_PATH];
                sprintf(file_content_path, "%s\\%s", conteudo, content_hold);
                free(content_hold);

                CopyFile(file_content_path, dest_path, FALSE);
            }
        }
    } while(FindNextFile(hFind, &fd));

    FindClose(hFind);
}

int loadBuild(char* orig, char* dest, int ver){
    char salver_path[MAX_PATH];
    sprintf(salver_path, "%s\\build\\salver", orig);

    char build_path[MAX_PATH];
    if (ver >= 0){
        sprintf(build_path, "%s\\build\\%d", orig, ver);
    }
    else{
        char* content;
        size_t size = 0;
        FILE* f = fopen(salver_path, "rb");
        readFile(f, &size, &content);
        fclose(f);
        sprintf(build_path, "%s\\build\\%s", orig, content);
        free(content);
    }

    char conteudo_path[MAX_PATH];
    sprintf(conteudo_path, "%s\\conteudo", orig);

    if (GetFileAttributesA(build_path) != INVALID_FILE_ATTRIBUTES) {
        hashLoadBuild(build_path, dest, conteudo_path);
        return 0;
    }
    printf("Build nao existe.");
    return -1;
}


void listRegistros(){
    WIN32_FIND_DATA fd;

    char dirReg[MAX_PATH];
    sprintf(dirReg, "%s\\*", DEST);

    HANDLE hFind = FindFirstFile(dirReg, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")){
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                printf("Registro %s -> ", fd.cFileName);

                char dirSalver[MAX_PATH];
                sprintf(dirSalver, "%s\\%s\\build\\salver", DEST, fd.cFileName);

                char* content;
                size_t size = 0;
                FILE* f = fopen(dirSalver, "rb");
                readFile(f, &size, &content);
                fclose(f);

                printf("versão %s\n", content);

            } 
        }
    } while(FindNextFile(hFind, &fd));

    FindClose(hFind);
}


int main(int argc, char** argv){
    SetConsoleOutputCP(CP_UTF8);
    createCheckDir(DEST);

    if (argc == 1){
        listRegistros();
        return 0;
    }
    
    char ppath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, ppath);
    char regDir[MAX_PATH] = "";
    sprintf(regDir, "%s\\%s", DEST, argv[argc-1]);
    
    if (argc == 2){
        newBuild(ppath, regDir);
    }

    else{
        int modes = 0;
        int load_flag_i = 0;

        for (int i = 1; i < argc-1; i++){
            if ((startsWith(argv[i], "-new") == 1)) modes |= M_NEW;
            if ((startsWith(argv[i], "-load") == 1)) {modes |= M_LOAD; load_flag_i = i;}
            if ((startsWith(argv[i], "-exe") == 1)) modes |= M_EXE;
        }

        if ((modes & M_NEW) == M_NEW){
            criarDirRegistro(regDir);
            return 0;
        }

        if ((modes & M_LOAD) == M_LOAD){
            int n = -1;

            char verString[15];
            snprintf(verString, 15, "%s", argv[load_flag_i]+5);
            if (verString[0] != '\0') n = atoi(verString);

            loadBuild(regDir, ppath, n);
            
            return 0;
        }

        if ((modes & M_EXE) == M_EXE){
            ignore_exes = 0;
        }

        newBuild(ppath, regDir);
    }

    return 0;
}