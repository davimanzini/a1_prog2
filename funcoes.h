#include <stdio.h>
#include <stdlib.h>

struct diretorio{

    char nome[1000];
    uid_t uid;
    long int tamanho_original;
    long int tamanho_disco;
    time_t data_mod;
    long int ordem;
    long int localizacao; //endereço do arquivo em archive
};
