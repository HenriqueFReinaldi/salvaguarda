#include "util.h"


//configurationes
#define BUILDER_NAME_SIZE 50
char BUILDER_NAME[BUILDER_NAME_SIZE];
#define BUILDER_MESSAGE_SIZE 300
char BUILDER_MESSAGE[BUILDER_MESSAGE_SIZE];


char DEST[MAX_PATH];
char REGISTRO_LASTARG[MAX_PATH];
Set ignore_file_types = {100, 0, NULL};
Set ignore_folders = {100, 0, NULL};

//---

#define M_NEW 0x01 //
#define M_LOAD 0x02 //carregar porraloca
#define M_EXE 0x04 //incluir .exes ao salvar
#define M_SPC 0x08 //criar build especial
#define M_VIEW 0x10 //visualizar builds do regis
#define M_CPYMSG 0x20 // mostrar mensagems de copiar

//dirs
int createCheckDir(char* dest){
    if (!(CreateDirectory(dest, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)){
        return -1;
    } 
    return 0;
}

void criarDirRegistro(char* path){
    if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
        printf("Registro ja existe.");
        return;
    }

    if(!CreateDirectory(path, NULL)){
        printf("erro");
        return;
    }
    char contentPath[MAX_PATH];
    char buildPath[MAX_PATH];
    snprintf(contentPath, MAX_PATH, "%s\\conteudo", path);
    snprintf(buildPath, MAX_PATH, "%s\\build", path);

    if(!CreateDirectory(contentPath, NULL)){
        printf("erro");
        return;
    }
    if(!CreateDirectory(buildPath, NULL)){
        printf("erro");
        return;
    }

    snprintf(buildPath, MAX_PATH,"%s\\build\\salver", path);

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


//builds
int ignore_exes = 1;
int copy_messages = 0;

int file_count;
int files_done = 0;
int files_ignored = 0;
int new_files = 0;

int getFileHash(FILE* f, char hash[41]){
    static size_t size = 0;

    static size_t content_size = 0;
    static char* content;

    int result = staticGrowReadFile(f, &size, &content, &content_size);

    // printf("\n\n%s conteudo:%lld tamanhodobuffer:%lld [%d] \n\n", content, content_size, size, result);
    // char c;
    // scanf("%c", &c);


    unsigned char out[SHA_DIGEST_LENGTH]; 
    SHA1((unsigned char*)content, content_size, out);

    for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(hash+i*2, "%02x", out[i]);
    }
    return 0;
}


void hashCLBuild(char* orig, char* dest, char* conteudo, int copy){
    WIN32_FIND_DATA fd;
    char fonte_path[MAX_PATH];
    char dest_path[MAX_PATH];
    CreateDirectory(dest, NULL);

    snprintf(fonte_path, MAX_PATH, "%s\\*", orig);
    HANDLE hFind = FindFirstFile(fonte_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")){
            snprintf(fonte_path, MAX_PATH, "%s\\%s", orig, fd.cFileName);
            snprintf(dest_path, MAX_PATH, "%s\\%s", dest, fd.cFileName);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                if (!contains(ignore_folders, fd.cFileName)) hashCLBuild(fonte_path, dest_path, conteudo, copy);
                else{
                    int ignored = fileTravel(fonte_path, 1, 0, 0);
                    files_ignored += ignored;
                    files_done += ignored;
                }
            }
            else if (copy){
                int can_copy = 1;

                char* ext_dot = strrchr(fd.cFileName, '.');
                if (ext_dot != NULL && contains(ignore_file_types, ext_dot)) can_copy = 0;

                if (can_copy){
                    FILE* f = fopen(fonte_path, "rb");
                    char hash[41] = {0};
                    getFileHash(f, hash);
                    fclose(f);

                    char file_path[MAX_PATH];
                    snprintf(file_path, MAX_PATH, "%s\\%s",conteudo, hash);

                    DWORD at = GetFileAttributesA(file_path);
                    if(!(at != INVALID_FILE_ATTRIBUTES && !(at & FILE_ATTRIBUTE_DIRECTORY))){
                        CopyFile(fonte_path, file_path, FALSE);
                        new_files++;
                        if (copy_messages) printf("Copiado : %s -> %s\n", fonte_path, file_path);
                    }

                    f = fopen(dest_path, "wb");
                    writeFile(f, hash);
                    fclose(f);
                }
                else{
                    if (copy_messages) printf(RED "Ignorado: " RESET "%s\n", fonte_path);
                    files_ignored++;
                }
                files_done++;
                if (!copy_messages) printf("Progresso:" BLUE " %d\% (%d/%d)\r" RESET, (int)(((float)files_done/file_count)*100),files_done, file_count);
            }
            else{
                size_t size = 50;
                char content_name[size];
                
                FILE* f = fopen(fonte_path, "rb");
                noMallocReadFile(f, size, content_name);
                fclose(f);

                char file_content_path[MAX_PATH];
                snprintf(file_content_path, MAX_PATH, "%s\\%s", conteudo, content_name);

                CopyFile(file_content_path, dest_path, FALSE);
            }

        }
    } while(FindNextFile(hFind, &fd));

    FindClose(hFind);
}

    //create
int newBuild(char* orig, char* dest, char* nome){
    if(GetFileAttributesA(dest) == INVALID_FILE_ATTRIBUTES){
        printf("Registro não existe.");
        return -1;
    }

    char buildPath[MAX_PATH];
    char conteudoPath[MAX_PATH];
    char logsPath[MAX_PATH];
    char newVerPath[MAX_PATH];
    snprintf(buildPath, MAX_PATH, "%s\\build", dest);
    snprintf(conteudoPath, MAX_PATH, "%s\\conteudo", dest);
    snprintf(logsPath, MAX_PATH, "%s\\logs.txt", dest);

    if (nome){
        snprintf(newVerPath, MAX_PATH, "%s\\%s", buildPath, nome);

        if (GetFileAttributesA(newVerPath) != INVALID_FILE_ATTRIBUTES) {
            printf("Build ja existe.");
            return -1;
        }
    }
    else{
        char* last_slash_orig = strrchr(orig, '\\');
        char orig_folder_name[MAX_PATH];
        snprintf(orig_folder_name, MAX_PATH, last_slash_orig+1);

        if (strcmp(orig_folder_name, REGISTRO_LASTARG) != 0){
            char resposta;
            printf("Mandar '" RED "%s" RESET "' para '" RED "%s" RESET"'? (s/n): ", orig_folder_name, REGISTRO_LASTARG);
            scanf("%c", &resposta);
            if (resposta != 's' && resposta != 'S') return 0;
        }


        char salverPath[MAX_PATH];
        snprintf(buildPath, MAX_PATH, "%s\\build", dest);
        snprintf(conteudoPath, MAX_PATH, "%s\\conteudo", dest);
        snprintf(salverPath, MAX_PATH, "%s\\salver", buildPath);

        char* content;
        size_t content_size = 0;

        FILE* salver_file = fopen(salverPath, "rb");
        readFile(salver_file, &content_size, &content);
        fclose(salver_file);
        int current_ver = atoi(content)+1;

        snprintf(newVerPath, MAX_PATH, "%s\\%d", buildPath, current_ver);

        char new_ver[30];
        snprintf(new_ver, (size_t)30, "%d", current_ver);
        
        salver_file = fopen(salverPath, "wb");
        writeFile(salver_file, new_ver);
        fclose(salver_file);
    }

    file_count = fileTravel(orig, 1, 0, 0);
    
    printf("Nova build: %s\n",newVerPath);
    printf("Arquivos a serem processados: %d\n", file_count);
    hashCLBuild(orig, newVerPath, conteudoPath, 1);
    printf("\nArquivos novos: "BLUE "%d\n" RESET, new_files);
    printf("Arquivos ignorados: "RED "%d\n" RESET, files_ignored);

    time_t agora = time(NULL);
    struct tm* tm_info = localtime(&agora);


    char log_buffer[MAX_PATH*3];
    snprintf(log_buffer, MAX_PATH*3, 
        "%d/%02d/%02d - %02d:%02d:%02d:\n"
        "    nova build: %s\n"
        "    builder: %s\n"
        "    mensagem: %s\n\n",
        tm_info->tm_mday, tm_info->tm_mon+1, tm_info->tm_year+1900, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, 
        newVerPath, 
        BUILDER_NAME,
        BUILDER_MESSAGE
    );

    FILE* logs_file = fopen(logsPath, "a");
    if (logs_file) writeFile(logs_file, log_buffer);
    fclose(logs_file);
    return 0;
}

    //load
int loadBuild(char* orig, char* dest, char* ver){
    if(GetFileAttributesA(orig) == INVALID_FILE_ATTRIBUTES){
        printf("Registro não existe.");
        return -1;
    }

    char salver_path[MAX_PATH];
    snprintf(salver_path, MAX_PATH, "%s\\build\\salver", orig);

    char build_path[MAX_PATH];
    if (strlen(ver) != 0){
        snprintf(build_path, MAX_PATH, "%s\\build\\%s", orig, ver);
    }
    else{
        char* content;
        size_t size = 0;
        FILE* f = fopen(salver_path, "rb");
        readFile(f, &size, &content);
        fclose(f);
        snprintf(build_path, MAX_PATH, "%s\\build\\%s", orig, content);
        free(content);
    }

    char conteudo_path[MAX_PATH];
    snprintf(conteudo_path, MAX_PATH, "%s\\conteudo", orig);

    if (GetFileAttributesA(build_path) != INVALID_FILE_ATTRIBUTES) {
        hashCLBuild(build_path, dest, conteudo_path, 0);
        return 0;
    }
    printf("Build nao existe.");
    return -1;
}


//visualizacao
void listRegistros(){
    WIN32_FIND_DATA fd;

    char dirReg[MAX_PATH];
    snprintf(dirReg, MAX_PATH, "%s\\*", DEST);

    HANDLE hFind = FindFirstFile(dirReg, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")){
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                printf("Registro "BLUE "%s" RESET " -> ", fd.cFileName);

                char dirSalver[MAX_PATH];
                snprintf(dirSalver, MAX_PATH, "%s\\%s\\build\\salver", DEST, fd.cFileName);

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

//Prog
void init(){
    SetConsoleOutputCP(CP_UTF8);
    initSet(&ignore_file_types);
    initSet(&ignore_folders);

    snprintf(BUILDER_NAME, BUILDER_NAME_SIZE, "nem deus sabe");
    
    char program_path[MAX_PATH]; 
    GetModuleFileNameA(NULL, program_path, MAX_PATH);
    *strrchr(program_path, '\\') = '\0';

    char config_path[MAX_PATH];
    snprintf(config_path, MAX_PATH, "%s\\svconfig.txt", program_path);

    char** svconfig_lines;
    size_t line_count;

    FILE* f = fopen(config_path, "rb");
    if (!f) msgExit("Sem arquivo svconfig...");

    getFileLines(f, &svconfig_lines, &line_count);

    fclose(f);

    if (line_count > 0){
        strcpy(DEST, svconfig_lines[0]);
    }
    for (size_t i = 1; i < line_count; i++){
        size_t arg_length = strlen(svconfig_lines[i]);
        if (arg_length <= 8) continue;

        char* resto = svconfig_lines[i]+8;

        if (startsWith(svconfig_lines[i], "dignore ") == 1){
            addKey(&ignore_folders, resto);
        }
        else if (startsWith(svconfig_lines[i], "tignore ") == 1){
            addKey(&ignore_file_types, resto);
        }
        else if (startsWith(svconfig_lines[i], "builder ") == 1){
            snprintf(BUILDER_NAME, BUILDER_NAME_SIZE, "%s", resto);
        }
    }

    if (createCheckDir(DEST) != 0) msgExit("Falha em criar / achar diretorio destino (primeira linha svconfig)");
}

int main(int argc, char** argv){
    char proj_path[MAX_PATH];
    char reg_path[MAX_PATH];

    init();

    GetCurrentDirectoryA(MAX_PATH, proj_path);
    snprintf(reg_path, MAX_PATH, "%s\\%s", DEST, argv[argc-1]);
    snprintf(REGISTRO_LASTARG, MAX_PATH, "%s", argv[argc-1]);

    if (argc == 1){
        printf(BLUE "salvaguarda " RESET "versão " BLUE "%s " RESET "windows\n", VER);
        printf("digite '"BLUE "salt -ajuda" RESET"' para saber mais\n\n");
        listRegistros();    
        return 0;
    }    
    else if (argc == 2){
        if (strcmp(argv[argc-1], "-ajuda") == 0){
            intro();
            return 0;
        }

        newBuild(proj_path, reg_path, NULL);
        return 0;
    }

    int flags = 0;
    int special_flag_index = 0;
    int load_flag_index = 0;
    
    for (int i = 1; i < argc-1; i++){
        //muda algo
        if ((startsWith(argv[i], "-from") == 1)){
            char new_path[MAX_PATH];
            snprintf(new_path, MAX_PATH, "%s", argv[i]+5);
            DWORD fa = GetFileAttributes(new_path);
            if (fa == INVALID_FILE_ATTRIBUTES || ((fa & FILE_ATTRIBUTE_DIRECTORY) == 0)){
                printf("caminho indevido / inexistente.");
                return 0;
            }
            snprintf(proj_path, MAX_PATH, "%s", new_path);
        }
        else if ((startsWith(argv[i], "-msg") == 1)){
            snprintf(BUILDER_MESSAGE, BUILDER_MESSAGE_SIZE, "%s", argv[i]+4);
        }


        //termina programa
        else if ((startsWith(argv[i], "-new") == 1)){
            criarDirRegistro(reg_path);
            return 0;
        }
        else if ((startsWith(argv[i], "-view") == 1)){
            char build_path[MAX_PATH];
    
            if (argv[i][5] != '\0'){
                snprintf(build_path, MAX_PATH, "%s\\build\\%s", reg_path, argv[i]+5);
                printf("\n");
                fileTravel(build_path, 1, 1, 1);
                return 0;
            }

            snprintf(build_path, MAX_PATH, "%s\\build", reg_path, argv[i]+5);
            fileTravel(build_path, 0, 0 ,1);
            return 0;
        }
        
        
        //levanta flag
        else if ((startsWith(argv[i], "-load") == 1)){
            load_flag_index = i;
            flags |= M_LOAD;
        } 
        else if ((startsWith(argv[i], "-esp") == 1)){
            special_flag_index = i;
            flags |= M_SPC;
        }
        else if ((startsWith(argv[i], "-info") == 1)) flags |= M_CPYMSG;
    }
    //flags |= M_CPYMSG;
    //flags exclusivas
    if ((flags & M_LOAD) == M_LOAD){
        char ver_string[50];
        snprintf(ver_string, (size_t)50, "%s", argv[load_flag_index]+5);

        loadBuild(reg_path, proj_path, ver_string);
        return 0;
    }


    //nao exclusivas
    if ((flags & M_CPYMSG) == M_CPYMSG){
        copy_messages = 1;
    }
    if ((flags & M_SPC) == M_SPC){
        char nome_build[50];
        snprintf(nome_build, (size_t)50, "%s", argv[special_flag_index]+4);
        newBuild(proj_path, reg_path, nome_build);
        return 0;
    }
    newBuild(proj_path, reg_path, NULL);

    return 0;
}