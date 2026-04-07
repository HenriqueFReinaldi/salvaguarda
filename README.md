
# salvaguarda

É uma ferramenta extremamente simples que criei para me ajudar na criação de registros do estado de projetos no Windows.

### uso:
  
    salt {registro}                 | Cria uma nova build e manda pro registro.
    salt -ajuda                     | Mostra esta lista.
    
    salt -new {registro}            | Cria um novo registro.
    salt -load{n} {registro}        | Carrega a última versão (ou uma versão {n}) do registro pro diretório atual.
    salt -from{d} {registro}        | Troca o diretório de execução para {d}.
    
    salt -view{build} {registro}    | Mostra todas as builds (ou uma determinada) do registro.
    salt -msg {registro}            | Mostra mensagens de copiar.
    salt -esp{nome} {registro}      | Cria uma build com nome especial.

    Nota: não inclua as chaves nos comandos.

    Inclua o comando 'tignore {.tipo}' no arquivo svconfig.txt para excluir tipos de arquivos das builds.
    Inclua o comando 'dignore {dir}' no arquivo svconfig.txt para excluir certos diretorios das builds.

### compilação:

  Requisitos:
    
    GCC 64 bits (x86_64)
    Biblioteca de desenvolvimento OpenSSL (libssl, libcrypto)
    
  Build:

    gcc sal.c util.c set/set.c -o sal -lssl -lcrypto
