#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> // o que faz
#include "funcoes.h"

void inserir_sem_compressao(char *archive, char **arquivos, int n){

    FILE *fp_archive = fopen(archive, "ab+"); //ab+ é leitura e escrita no final do arquivo

    for(int i; i < n; i++){

        FILE *fp_membro = fopen(arquivos[i], "rb");
    }
}
