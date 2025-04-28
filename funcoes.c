#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> // o que faz
#include "funcoes.h"


void inserir_sem_compressao(char *archive, char **arquivos, int n){ //n é numero de arquivos

    //CHECAR SE MEMBRO JA ESTA NO ARCHIVE!!

    FILE *fp_archive = fopen(archive, "rb+"); //ab+ é leitura e escrita no final do arquivo
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
        dir.data_mod         = time(NULL);
        dir.ordem            = i;
        dir.uid              = getuid(); //?
        dir.tamanho_original = tamanho;
        dir.tamanho_disco    = tamanho;
        dir.localizacao      = posicao;

        diretorios[i] = dir; //salva as infos do membro[i] no vet. de diretorios

    }

    for(int i = 0; i < n; i++){
        fwrite(&diretorios[i], sizeof(struct diretorio), 1, fp_archive); //?
    }

    fwrite(&n, sizeof(int), 1, fp_archive); //add int numero de intens no final do archive

    fclose(fp_archive);
}

void lista_informacoes(char *archive){ // -c

    FILE *fp_archive = fopen(archive, "rb"); //conferir se rb ta certo
    if(!fp_archive){
        perror("Erro ao abrir o archive");
        return;
    }

    fseek(fp_archive, 0, SEEK_END);
    long int tamanho = ftell(fp_archive);

    //caso do archive estar vazio
    if(tamanho <= 0){ //checar essa condicao
        perror("Erro: archive está vazio");
        fclose(fp_archive);
        return;
    }

    int qtd_arquivos;

    fseek(fp_archive, - sizeof(int), SEEK_END);
    fread(&qtd_arquivos, sizeof(int), 1, fp_archive); //duvida em passar & de qtd_arquivos
    
    fseek(fp_archive, - sizeof(int) - (qtd_arquivos * sizeof(struct diretorio)), SEEK_END); //coloca o ponteiro para o comeco dos diretorios
    
    struct diretorio dir;
    for(int i = 0; i < qtd_arquivos; i++){
        fread(&dir, sizeof(struct diretorio), 1, fp_archive);

        printf("Nome do arquivo: %s\n", dir.nome); // qnd usar ponto e quando usar seta?
        printf("UID: %d\n", dir.uid);
        printf("Tamanho original: %ld\n", dir.tamanho_original);
        printf("Tamanho em disco: %ld\n", dir.tamanho_disco);
        printf("Data de modificacao: %ld\n", dir.data_mod); //formatar? data_mod é long int??
        printf("Ordem no archive: %ld\n", dir.ordem);
        printf("Localizacao no archive: %ld\n", dir.localizacao);
    }

    fclose(fp_archive);

    return;
    
}
