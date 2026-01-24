
# salvaguarda

É uma ferramenta extremamente simples que criei para me ajudar na criação de registros do estado de projetos no Windows.

### uso:
  
    salt                      | mostra todos os registros.
    
    salt salvaguarda          | salva diretório pro registro.
    
    salt -new salvaguarda     | cria novo registro.
    salt -load salvaguarda    | carrega ultima versão do registro.
    salt -load{n} salvaguarda | carrega versão {n} do registro. 
    
    salt -exe salvaguarda     | salva também os arquivos .exe.

### compilação:

  Requisitos:
    
    GCC 64 bits (x86_64)
    Biblioteca de desenvolvimento OpenSSL (libssl, libcrypto)
    
  Build:

    gcc sal.c readwrite.c -o sal -lssl -lcrypto
