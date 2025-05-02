#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "funcoes.h"


int main(int argc, char *argv[]){

    if(argc < 3){
        perror("Argumentos insuficientes");
        return -1;
    }

    int qtd_arquivos = argc - 3; //checar
    char *arquivos[qtd_arquivos];

    for(int i = 0; i < qtd_arquivos; i++){
        arquivos[i] = argv[i + 3];
    }

    int opcao = -1;
    
    if(strcmp(argv[1], "-ip") == 0)
        opcao = 1;
    else if(strcmp(argv[1], "-ic") == 0)
        opcao = 2;
    else if(strcmp(argv[1], "-m") == 0)
        opcao = 3;
    else if(strcmp(argv[1], "-x") == 0)
        opcao = 4;
    else if(strcmp(argv[1], "-r") == 0)
        opcao = 5;
    else if(strcmp(argv[1], "-c") == 0)
        opcao = 6;

    switch(opcao){

        case 1:
            printf("Executando insercao sem compressao (-ip)!\n");
            inserir_sem_compressao(argv[2], arquivos, qtd_arquivos);
            break;
        
        case 2:

            break;
        
        case 3:

            break;
        
        case 4:

            break;
        
        case 5:

            break;

        case 6:
            printf("Executando funcao de listagem (-c)!\n");
            lista_informacoes(argv[2]);
            break;
        
        default:
            printf("Opcao invalida!\n");
            return -1;
    }

    return 0;
}