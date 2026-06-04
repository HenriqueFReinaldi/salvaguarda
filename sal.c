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

void criarDirRegistro(char* path){
    if (path_acessible(path)) {
        printf("Registro ja existe.\n");
        return;
    }

    if(!createdir(path)){
        printf("erro");
        return;
    }
    char contentPath[MAX_PATH];
    char buildPath[MAX_PATH];
    snprintf(contentPath, MAX_PATH, "%s" PATH_SEP_STR "conteudo", path);
    snprintf(buildPath, MAX_PATH, "%s" PATH_SEP_STR "build", path);

    if(!createdir(contentPath)){
        printf("erro");
        return;
    }
    if(!createdir(buildPath)){
        printf("erro");
        return;
    }

    snprintf(buildPath, MAX_PATH,"%s" PATH_SEP_STR "build" PATH_SEP_STR "salver", path);

    FILE* f = fopen(buildPath, "w");
    if (!f){
        printf("erro");
        return;
    }
    fprintf(f, "0");
    fclose(f);


    printf("Criado novo registro: %s\n", path);
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

void hashLoadBuild(char* orig, char* dest, char* cont_path){
    char fonte_path[MAX_PATH];
    char dest_path[MAX_PATH];
    createdir(dest);

    DirIt* p = init_dirit(orig, 1);

    int path_length = strlen(orig);
    int r;
    while(r = path_travel(&p)){
        if (r == 2) continue;

        snprintf(dest_path, MAX_PATH, "%s%s", dest, p->item_path+path_length);
        if (p->is_file){
            snprintf(fonte_path, MAX_PATH, "%s" PATH_SEP_STR "%s", p->path, p->item_name);
            
            char file_hash[50];
            FILE* f = fopen(fonte_path, "rb");
            noMallocReadFile(f, 50, file_hash);
            fclose(f);

            char file_content_path[MAX_PATH];
            snprintf(file_content_path, MAX_PATH, "%s" PATH_SEP_STR "%s", cont_path, file_hash);

            copy_file(file_content_path, dest_path);
        }
        else{
            createdir(dest_path);
        }


    }

}

void hashCopyBuild(char* orig, char* dest, char* cont_path){
    char fonte_path[MAX_PATH];
    char dest_path[MAX_PATH];
    createdir(dest);
    DirIt* p = init_dirit(orig, 1);

    int path_length = strlen(orig);
    int r;
    while(r = path_travel(&p)){
        if (r == 2) continue;

        snprintf(dest_path, MAX_PATH, "%s%s", dest, p->item_path+path_length);
        if (p->is_file){
            snprintf(fonte_path, MAX_PATH, "%s" PATH_SEP_STR "%s", p->path, p->item_name);
            char* ext_dot = strrchr(p->item_name, '.');
            if ((ext_dot != NULL && contains(ignore_file_types, ext_dot))){
                if (copy_messages) printf(COR_NOK "Ignorado" RESET ": %s\n", fonte_path);
                files_ignored++;
                goto progresso;
            } 

            //copiar
            FILE* f = fopen(fonte_path, "rb");
            char hash[41] = {0};
            getFileHash(f, hash);
            fclose(f);

            char file_path[MAX_PATH];
            snprintf(file_path, MAX_PATH, "%s" PATH_SEP_STR "%s", cont_path, hash);

            if(!path_acessible(file_path) || is_folder(file_path)){
                new_files++;

                copy_file(fonte_path, file_path);
                if (copy_messages) printf("Copiado : %s -> %s\n", fonte_path, file_path);
            }
            f = fopen(dest_path, "wb");
            writeFile(f, hash);
            fclose(f);

            progresso:
            files_done++;
            if (!copy_messages) printf("Progresso:" COR_OK " %d%% (%d/%d)\r" RESET, (int)(((float)files_done/file_count)*100),files_done, file_count);
        }
        else {
            if (contains(ignore_folders, p->item_name)){
                int ignored = count_files(p->path, 1);
                printf(COR_NOK "Ignorados" RESET " [%d] de %s\n", ignored, p->path);
                files_ignored += ignored;
                files_done += ignored;

                skip_folder(&p);
                continue;
            }
            createdir(dest_path);
        }
    }
}

    //create
int newBuild(char* orig, char* dest, char* nome){
    if(!path_acessible(dest)){
        printf("Registro não existe.\n");
        return -1;
    }

    char buildPath[MAX_PATH];
    char conteudoPath[MAX_PATH];
    char logsPath[MAX_PATH];
    char newVerPath[MAX_PATH];
    snprintf(buildPath, MAX_PATH, "%s" PATH_SEP_STR "build", dest);
    snprintf(conteudoPath, MAX_PATH, "%s" PATH_SEP_STR "conteudo", dest);
    snprintf(logsPath, MAX_PATH, "%s" PATH_SEP_STR "logs.txt", dest);

    if (nome){
        snprintf(newVerPath, MAX_PATH, "%s" PATH_SEP_STR "%s", buildPath, nome);

        if (path_acessible(newVerPath)) {
            printf("Build ja existe.\n");
            return -1;
        }
    }
    else{
        char* last_slash_orig = strrchr(orig, PATH_SEP);
        char orig_folder_name[MAX_PATH];
        snprintf(orig_folder_name, MAX_PATH, "%s", last_slash_orig+1);

        if (strcmp(orig_folder_name, REGISTRO_LASTARG) != 0){
            char resposta;
            printf("Mandar [" COR_OK "%s" RESET "] para [" COR_OK "%s" RESET"] (s/n): ", orig_folder_name, REGISTRO_LASTARG);
            scanf("%c", &resposta);
            if (resposta != 's' && resposta != 'S') return 0;
        }


        char salverPath[MAX_PATH];
        (void)snprintf(buildPath, MAX_PATH, "%s" PATH_SEP_STR "build", dest);
        (void)snprintf(conteudoPath, MAX_PATH, "%s" PATH_SEP_STR "conteudo", dest);
        (void)snprintf(salverPath, MAX_PATH, "%s" PATH_SEP_STR "salver", buildPath);

        char* content;
        size_t content_size = 0;

        FILE* salver_file = fopen(salverPath, "rb");
        readFile(salver_file, &content_size, &content);
        fclose(salver_file);
        int current_ver = atoi(content)+1;

        snprintf(newVerPath, MAX_PATH, "%s" PATH_SEP_STR "%d", buildPath, current_ver);

        char new_ver[30];
        snprintf(new_ver, (size_t)30, "%d", current_ver);
        
        salver_file = fopen(salverPath, "wb");
        writeFile(salver_file, new_ver);
        fclose(salver_file);
    }

    file_count = count_files(orig, 1);
    
    printf("Nova build: %s\n",newVerPath);
    printf("Arquivos a serem processados: %d\n", file_count);

    hashCopyBuild(orig, newVerPath, conteudoPath);

    printf("\nArquivos novos: "COR_OK "%d\n" RESET, new_files);
    printf("Arquivos ignorados: "COR_OK "%d\n" RESET, files_ignored);
    printf("Arquivos duplicados: "COR_OK "%d" RESET"\n",  file_count-(new_files+files_ignored));

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
    if(!path_acessible(orig)){
        printf("Registro não existe.\n");
        return -1;
    }

    char salver_path[MAX_PATH];
    snprintf(salver_path, MAX_PATH, "%s" PATH_SEP_STR "build" PATH_SEP_STR "salver", orig);

    char build_path[MAX_PATH];
    if (strlen(ver) != 0){
        snprintf(build_path, MAX_PATH, "%s" PATH_SEP_STR "build" PATH_SEP_STR "%s", orig, ver);
    }
    else{
        char* content;
        size_t size = 0;
        FILE* f = fopen(salver_path, "rb");
        readFile(f, &size, &content);
        fclose(f);
        snprintf(build_path, MAX_PATH, "%s" PATH_SEP_STR "build" PATH_SEP_STR "%s", orig, content);
        free(content);
    }

    char conteudo_path[MAX_PATH];
    snprintf(conteudo_path, MAX_PATH, "%s" PATH_SEP_STR "conteudo", orig);

    if (!path_acessible(build_path)) {
        printf("Build nao existe.");
        return -1;
    }

    hashLoadBuild(build_path, dest, conteudo_path);
    
    printf("[" COR_OK "%s" RESET "] enviado para [" COR_OK "%s" RESET"]\n", orig, dest);
    return 0;
}


//visualizacao
void listRegistros(){
    DirIt* p = init_dirit(DEST, 0);

    int count = 0;
    int r;
    while(r = path_travel(&p)){
        if (r == 2) continue;
       
        if (!p->is_file){
            printf("Registro "COR_OK "%s" RESET " ", p->item_name);

            char path_salver[MAX_PATH];
            snprintf(path_salver, MAX_PATH, "%s" PATH_SEP_STR "build" PATH_SEP_STR "salver", p->item_path);

            char* content;
            size_t size = 0;
            FILE* f = fopen(path_salver, "rb");
            readFile(f, &size, &content);
            if (f) fclose(f);

            printf("v.%s\n", content);
            count++;
        }
    }
    printf("\nRegistros: %d\n", count);

}

//Prog
void init(){
    initSet(&ignore_file_types);
    initSet(&ignore_folders);

    snprintf(BUILDER_NAME, BUILDER_NAME_SIZE, "nem deus sabe");
    
    char program_path[MAX_PATH]; 
    get_program_path(program_path);

    *strrchr(program_path, PATH_SEP) = '\0';

    char config_path[MAX_PATH];
    snprintf(config_path, MAX_PATH, "%s" PATH_SEP_STR "svconfig.txt", program_path);

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
    if (!path_acessible(DEST)){
        if(!createdir(DEST)){
            msgExit("Falha em criar / achar diretorio destino (primeira linha svconfig)");
        }
    }
}

int main(int argc, char** argv){
    char proj_path[MAX_PATH];
    char reg_path[MAX_PATH];

    init();

    get_working_path(proj_path);

    snprintf(reg_path, MAX_PATH, "%s" PATH_SEP_STR "%s", DEST, argv[argc-1]);
    snprintf(REGISTRO_LASTARG, MAX_PATH, "%s", argv[argc-1]);

    if (argc == 1){
        printf(COR_OK "salvaguarda " RESET "versão " COR_OK "%s " RESET PROG_OS "\n", VER);
        printf("digite '"COR_OK "salt -ajuda" RESET"' para saber mais\n\n");
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

            if (!path_acessible(new_path) || !is_folder(new_path)){
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
                snprintf(build_path, MAX_PATH, "%s" PATH_SEP_STR "build" PATH_SEP_STR "%s", reg_path, argv[i]+5);
                printf("\n");
                display_dir(build_path, 1, 1, 1);
                return 0;
            }

            snprintf(build_path, MAX_PATH, "%s" PATH_SEP_STR "build", reg_path);
            display_dir(build_path, 0, 0 ,1);
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