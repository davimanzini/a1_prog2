#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> // o que faz
#include "funcoes.h"


void inserir_sem_compressao(char *archive, char **arquivos, int n){ //n é numero de arquivos

    //CHECAR SE MEMBRO JA ESTA NO ARCHIVE!!

    FILE *fp_archive = fopen(archive, "rb+"); //qqr mode com r só funciona se o arquivo ja existir
    if(!fp_archive){
        fp_archive = fopen(archive, "wb+"); //nao podemos usar só wb pq esse mode apaga tudo do arquivo
        if(!fp_archive){
            perror("Erro ao criar o archive");
            return;
        }
    }

    fseek(fp_archive, 0, SEEK_END);
    long int tamanho_total = ftell(fp_archive);

    int qtd_membros = 0;

    if(tamanho_total == 0){ //archive vazio!!

        struct membro diretorio[n];
    
        for(int i = 0; i < n; i++){

            FILE *fp_membro = fopen(arquivos[i], "rb"); //como é feita conexao nomearquivo e arquivo?
            if(!fp_membro){
                perror("Erro ao a abrir membro");
                continue; // pq continue mesmo?
            }

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

            struct membro mb;

            strcpy(mb.nome, arquivos[i]);
            mb.data_mod         = time(NULL);
            mb.ordem            = i;
            mb.uid              = getuid(); //?
            mb.tamanho_original = tamanho;
            mb.tamanho_disco    = tamanho;
            mb.localizacao      = posicao;

            diretorio[i] = mb; //salva as infos do membro[i] no vet. de diretorios

        }

        for(int i = 0; i < n; i++){
            fwrite(&diretorio[i], sizeof(struct membro), 1, fp_archive); //?
        }

        fwrite(&n, sizeof(int), 1, fp_archive); //add int numero de intens no final do archive

    }



    else if(tamanho_total != 0){ //archive nao esta vazio

        fseek(fp_archive, - sizeof(int), SEEK_END); //cm nao esta vazio, o ultimo item eh um int tamanho
        fread(&qtd_membros, sizeof(int), 1, fp_archive); //guarda qnts arquivos tem
        fseek(fp_archive, - sizeof(int) - (qtd_membros * sizeof(struct membro)), SEEK_END); //coloca ptr no cmc do diretorio

        struct membro dir[qtd_membros];

        for(int i = 0; i < qtd_membros; i ++){ //colocamos os membros do archive dentro de um vetor de membros (dir)
            fread(&dir[i], sizeof(struct membro), 1, fp_archive);
        }

        for(int i = 0; i < n; i++){

            int iguais = 0;

            for(int j = 0; j < qtd_membros; j++){
                if(strcmp(arquivos[i], dir[j].nome) == 0){ //achou um arquivo no archive com nome igual
                    iguais = 1;
                    break; //sai do for de dentro 
                }
               
            }

            if(iguais == 1){ //encontrou o arquivo no archive

            }

            else{ //nao encontrou o arquivo no archive

            }
        }



    }

    fclose(fp_archive); //fechamos o archive apenas após os dois condicionais (if/else)
}


//separacao de funcoes


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
    
    fseek(fp_archive, - sizeof(int) - (qtd_arquivos * sizeof(struct membro)), SEEK_END); //coloca o ponteiro para o comeco dos diretorios
    
    struct membro dir;
    for(int i = 0; i < qtd_arquivos; i++){
        fread(&dir, sizeof(struct membro), 1, fp_archive);

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
