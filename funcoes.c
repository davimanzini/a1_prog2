#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> // o que faz
#include "funcoes.h"

void inserir_sem_compressao(char *archive, char **arquivos, int n){ //n é numero de arquivos

    FILE *fp_archive = fopen(archive, "ab+"); //ab+ é leitura e escrita no final do arquivo

    for(int i; i < n; i++){

        FILE *fp_membro = fopen(arquivos[i], "rb"); //como é feita conexao nomearquivo e arquivo?

        fseek(fp_membro, 0, SEEK_END);
        long int tamanho = ftell(fp_membro);
        fseek(fp_membro, 0, SEEK_SET); //volta o ponteiro para o inicio do arquivo
        char *buffer = malloc(tamanho);
        fread(buffer, 1, tamanho, fp_membro);

    }
}
