#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/types.h>

struct membro{

    char nome[1000];
    uid_t uid;
    long int tamanho_original;
    long int tamanho_disco;
    time_t data_mod;
    long int ordem;
    long int localizacao; //endereço do arquivo em archive
};

void mover(FILE *arquivo, long int inicio, long int insercao, unsigned long tamanho);

void insere_sem_compressao(char *archive, char **arquivos, int n);

void lista_informacoes(char *archive);

void remove_arquivos(char *archive, char **arquivos, int n);

void extrai_arquivos(char *archive, char **arquivos, int n);
