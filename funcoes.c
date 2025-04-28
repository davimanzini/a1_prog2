#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> // o que faz
#include "funcoes.h"


void inserir_sem_compressao(char *archive, char **arquivos, int n){ //n é numero de arquivos

    //CHECAR SE MEMBRO JA ESTA NO ARCHIVE

    FILE *fp_archive = fopen(archive, "ab+"); //ab+ é leitura e escrita no final do arquivo
    if(!fp_archive){
        perror("Erro ao abrir o archive");
        return;
    }

    struct diretorio diretorios[n];
    
    for(int i = 0; i < n; i++){

        FILE *fp_membro = fopen(arquivos[i], "rb"); //como é feita conexao nomearquivo e arquivo?

        fseek(fp_membro, 0, SEEK_END);
        long int tamanho = ftell(fp_membro);
        fseek(fp_membro, 0, SEEK_SET); //volta o ponteiro para o inicio do arquivo
        
        char *buffer = malloc(tamanho); //reserva espaço temp. na RAM
        if(!buffer){
            perror("Erro ao abrir o arquivo");
            continue;
        }
        
        fread(buffer, 1, tamanho, fp_membro); //le do membro pro buffer
        long int posicao = ftell(fp_archive); //para saber em que posicao do archive o novo arq cmc
        fwrite(buffer, 1, tamanho, fp_archive); //escreve do buffer pro archive
        fclose(fp_membro);
        free(buffer);

        struct diretorio dir;

        strcpy(dir.nome, arquivos[i]);
        dir.data_mod = time(NULL);
        dir.ordem = i;
        dir.uid = getuid(); //?
        dir.tamanho_original = tamanho;
        dir.tamanho_disco = tamanho;
        dir.localizacao = posicao;

        diretorios[i] = dir; //salva as infos do membro[i] no vet. de diretorios

    }

    for(int i = 0; i < n; i++){
        fwrite(&diretorios[i], sizeof(struct diretorio), 1, fp_archive); //?
    }

    fclose(fp_archive);
}
