#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/types.h>

struct diretorio{

    char nome[1000];
    uid_t uid;
    long int tamanho_original;
    long int tamanho_disco;
    time_t data_mod;
    long int ordem;
    long int localizacao; //endereço do arquivo em archive
};


void inserir_sem_compressao(char *archive, char **arquivos, int n);

void preenche_diretorio(struct diretorio *dir, FILE *arquivo);


