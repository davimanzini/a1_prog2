#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> // o que faz
#include "funcoes.h"
#include "lz.h"

void mover(FILE *arquivo, long int inicio, long int insercao, unsigned long tamanho){ //pq unsigned?

    char *buffer = malloc(tamanho);

    fseek(arquivo, inicio, SEEK_SET);
    fread(buffer, tamanho, 1, arquivo);

    fseek(arquivo, insercao, SEEK_SET);
    fwrite(buffer, tamanho, 1, arquivo);

    free(buffer);
}


//separacao de funcoes


void inserir_sem_compressao(char *archive, char **arquivos, int n){ //n é numero de arquivos

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
        // struct membro *diretorio = malloc(n * sizeof(struct membro));

        for(int i = 0; i < n; i++){

            FILE *fp_membro = fopen(arquivos[i], "rb");
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
                fclose(fp_membro);
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


        struct membro dir[qtd_membros]; //debug

        for(int i = 0; i < qtd_membros; i ++){ //colocamos os membros do archive dentro de um vetor de membros (dir)
            fread(&dir[i], sizeof(struct membro), 1, fp_archive);
        }
    

        for(int i = 0; i < n; i++){

            int iguais = -1;

            for(int j = 0; j < qtd_membros; j++){
                if(strcmp(arquivos[i], dir[j].nome) == 0){ //achou um arquivo no archive com nome igual
                    iguais = j;
                    break; //sai do for de dentro 
                }
               
            }

            if(iguais != -1){ //encontrou o arquivo no archive

                FILE *fp_novo = fopen(arquivos[i], "rb");
                if(!fp_novo){
                    perror("Erro ao abrir o arquivo novo");
                    continue;
                }

                fseek(fp_novo, 0, SEEK_END);
                long int tam_insert = ftell(fp_novo); //tamanho do arq a ser inserido
                fseek(fp_novo, 0, SEEK_SET);
                
                if(tam_insert == dir[iguais].tamanho_disco){ //tamanhos iguais

                    char *buffer = malloc(tam_insert);

                    fread(buffer, 1, tam_insert, fp_novo);
                    fclose(fp_novo);

                    fseek(fp_archive, dir[iguais].localizacao, SEEK_SET);
                    fwrite(buffer, 1, tam_insert, fp_archive);
                    free(buffer);

                    dir[iguais].data_mod = time(NULL); //atualiza infos membro
                    dir[iguais].uid      = getuid();

                    fseek(fp_archive, 0, SEEK_END);
                    
                    long int pos_membros = ftell(fp_archive) - 
                    sizeof(int) - 
                    qtd_membros * (sizeof(struct membro));
                    
                    ftruncate(fileno(fp_archive), pos_membros); //realmente necessario?

                    fseek(fp_archive, 0, SEEK_END);
                    for(int k = 0; k < qtd_membros; k++){
                        fwrite(&dir[k], sizeof(struct membro), 1, fp_archive);
                    }

                    fwrite(&qtd_membros, sizeof(int), 1, fp_archive);
                }

                else if(tam_insert > dir[iguais].tamanho_disco){ //tamanho novo maior

                    long int tam_diff = tam_insert - dir[iguais].tamanho_disco;
                
                    fseek(fp_archive, - sizeof(int) - (qtd_membros * sizeof(struct membro)), SEEK_END);
                    long int pos_trunc = ftell(fp_archive);
                
                    ftruncate(fileno(fp_archive), pos_trunc);
                
                    for(int l = qtd_membros - 1; l > iguais; l--){ //movendo arquivos
                
                            mover(fp_archive,
                                dir[l].localizacao,
                                dir[l].localizacao + tam_diff,
                                dir[l].tamanho_disco);
                            
                                dir[l].localizacao += tam_diff;
                    }
                
                    char *buffer = malloc(tam_insert);
                    if(!buffer){
                        perror("Erro ao alocando buffer");
                        fclose(fp_novo);
                        continue;
                    }
                
                    fread(buffer, 1, tam_insert, fp_novo);
                    fclose(fp_novo);
                
                    fseek(fp_archive, dir[iguais].localizacao, SEEK_SET);
                    fwrite(buffer, 1, tam_insert, fp_archive);
                    free(buffer);
                
                    dir[iguais].tamanho_disco    = tam_insert;
                    dir[iguais].tamanho_original = tam_insert;
                    dir[iguais].data_mod         = time(NULL);
                    dir[iguais].uid              = getuid();
                
                    fseek(fp_archive, 0, SEEK_END);
                
                    for (int m = 0; m < qtd_membros; m++) {
                        fwrite(&dir[m], sizeof(struct membro), 1, fp_archive);
                    }
                
                    fwrite(&qtd_membros, sizeof(int), 1, fp_archive);
                }

                
                else if(tam_insert < dir[iguais].tamanho_disco){ //tamanho antigo maior

                    long int tam_diff = dir[iguais].tamanho_disco - tam_insert;

                    for(int k = 0; k < qtd_membros; k++){ //movendo arquivos

                        if(dir[k].localizacao > dir[iguais].localizacao){ //MUDAR QUE NEM O OUTRO FOR

                            mover(fp_archive,
                                dir[k].localizacao,
                                dir[k].localizacao - tam_diff,
                                dir[k].tamanho_disco);
                            
                                dir[k].localizacao -= tam_diff;
                        }
                    }

                    char *buffer = malloc(tam_insert);
                    if(!buffer){
                        perror("Erro ao alocando buffer");
                        fclose(fp_novo);
                        continue;
                    }

                    fread(buffer, 1, tam_insert, fp_novo);
                    fclose(fp_novo);

                    fseek(fp_archive, dir[iguais].localizacao, SEEK_SET);
                    fwrite(buffer, 1, tam_insert, fp_archive);
                    free(buffer);

                    dir[iguais].tamanho_disco    = tam_insert;
                    dir[iguais].tamanho_original = tam_insert;
                    dir[iguais].data_mod         = time(NULL);
                    dir[iguais].uid              = getuid();

                    fseek(fp_archive, - 
                        sizeof(int) - 
                        (qtd_membros * sizeof(struct membro)) - 
                        tam_diff, 
                        SEEK_END);
                    
                    long int pos_trunc = ftell(fp_archive);
                    
                    ftruncate(fileno(fp_archive), pos_trunc);

                    fseek(fp_archive, 0, SEEK_END);

                    for (int m = 0; m < qtd_membros; m++) {
                        fwrite(&dir[m], sizeof(struct membro), 1, fp_archive);
                    }

                    fwrite(&qtd_membros, sizeof(int), 1, fp_archive);
                }
            }

            else{ //nao encontrou o arquivo no archive

                FILE *novo_arquivo = fopen(arquivos[i], "rb+");
                if(!novo_arquivo){
                    perror("Erro ao abrir novo arquivo");
                    continue;
                }

                struct membro aux[qtd_membros + 1]; //CRIACAO DE VET AUXILIAR!!

                for(int k = 0; k < qtd_membros; k++){
                    aux[k] = dir[k];
                }

                fseek(novo_arquivo, 0, SEEK_END);
                long int tamanho = ftell(novo_arquivo); //salva o tamanho do novo arquivo
                fseek(novo_arquivo, 0, SEEK_SET);

                char *buffer = malloc(tamanho);
                if(!buffer){
                    perror("Erro alocando buffer");
                    fclose(novo_arquivo);
                    continue;
                }

                fread(buffer, 1, tamanho, novo_arquivo);
                fclose(novo_arquivo);

                fseek(fp_archive, 0, SEEK_END); 

                long int pos_insercao = ftell(fp_archive) - sizeof(int) - (qtd_membros * sizeof(struct membro));

                ftruncate(fileno(fp_archive), pos_insercao); //truncamos o arquivo, tiramos diretorio e int

                fseek(fp_archive, 0, SEEK_END);
                fwrite(buffer, tamanho, 1, fp_archive); //escreve novo arq no archive
                free(buffer);

                struct membro novo;

                strcpy(novo.nome, arquivos[i]);
                novo.data_mod         = time(NULL);
                novo.ordem            = qtd_membros;
                novo.uid              = getuid();
                novo.tamanho_original = tamanho;
                novo.tamanho_disco    = tamanho;
                novo.localizacao      = pos_insercao;

                aux[qtd_membros] = novo; //checar
                qtd_membros ++;

                for(int l = 0; l < qtd_membros; l++){
                    fwrite(&aux[l], sizeof(struct membro), 1, fp_archive);
                }

                fwrite(&qtd_membros, sizeof(int), 1, fp_archive);

            }
        }
    }

    fclose(fp_archive); //fechamos o archive apenas após os dois condicionais (if/else)
}


//separacao de funcoes


void lista_informacoes(char *archive){ // -c

    FILE *fp_archive = fopen(archive, "rb");
    if(!fp_archive){
        perror("Erro ao abrir o archive");
        return;
    }

    fseek(fp_archive, 0, SEEK_END);
    long int tamanho = ftell(fp_archive);

    if(tamanho <= 0){
        printf("Archive está vazio.\n");
        fclose(fp_archive);
        return;
    }

    int qtd_arquivos = 0;

    fseek(fp_archive, -sizeof(int), SEEK_END);
    if(fread(&qtd_arquivos, sizeof(int), 1, fp_archive) != 1){
        perror("Erro ao ler qtd_arquivos");
        fclose(fp_archive);
        return;
    }

    printf("Quantidade de arquivos no archive: %d\n\n", qtd_arquivos);

    fseek(fp_archive, -sizeof(int) - (qtd_arquivos * sizeof(struct membro)), SEEK_END);

    for(int i = 0; i < qtd_arquivos; i++){
        struct membro dir;
        if(fread(&dir, sizeof(struct membro), 1, fp_archive) != 1){  //precisa disso?
            perror("Erro ao ler struct membro");
            break;
        }

        char data_formatada[100];
        struct tm *info = localtime(&dir.data_mod);
        strftime(data_formatada, sizeof(data_formatada), "%d/%m/%Y %H:%M:%S", info);

        printf("Nome do arquivo: %s\n", dir.nome);
        printf("UID: %d\n", dir.uid);
        printf("Tamanho original: %ld\n", dir.tamanho_original);
        printf("Tamanho em disco: %ld\n", dir.tamanho_disco);
        printf("Data de modificacao: %s\n", data_formatada);
        printf("Ordem no archive: %ld\n", dir.ordem);
        printf("Localizacao no archive: %ld\n\n", dir.localizacao);
    }

    fclose(fp_archive);
}


void remove_arquivos(char *archive, char **arquivos, int n){

    FILE *fp_archive = fopen(archive, "rb+"); //checar mode
    if(!fp_archive){
        perror("Erro ao abrir o archive");
        return;
    }

    fseek(fp_archive, 0, SEEK_END);
    long int tam_archive = ftell(fp_archive);

    if (tam_archive == 0) { //archive vazio
        printf("Erro: o archive está vazio!\n");
        fclose(fp_archive);
        return;
    }

    else{ //archive nao esta vazio

        int qtd_membros;
        fseek(fp_archive, - sizeof(int), SEEK_END);
        fread(&qtd_membros, sizeof(int), 1, fp_archive);
        
        fseek(fp_archive, - 
            sizeof(int) - 
            (qtd_membros * sizeof(struct membro)), 
            SEEK_END);

        struct membro dir[qtd_membros];

        for(int i = 0; i < qtd_membros; i ++){ //colocamos os membros do archive dentro de um vetor de membros (dir)
            fread(&dir[i], sizeof(struct membro), 1, fp_archive);
        }

        fseek(fp_archive, 
            - sizeof(int) 
            - (qtd_membros * sizeof(struct membro)), 
            SEEK_END);

        long int pos_trunc = ftell(fp_archive); //truncar soh no final?

        ftruncate(fileno(fp_archive), pos_trunc); //trunca o diretorio e o count

        long int tam_trunc = 0; //damn

        for(int i = 0; i < n; i++){

            int iguais = -1;

            for(int j = 0; j < qtd_membros; j++){

                if(strcmp(arquivos[i], dir[j].nome) == 0){
                    iguais = j;
                    break;
                }
            }

            if(iguais == -1){ //nao achou no archive
                printf("Erro: esse arquivo nao esta no archive!\n");
                continue; //ta certo isso??
            }

            else if(iguais != -1){ //achou no archive

                for(int k = iguais + 1; k < qtd_membros; k++){
                    mover(fp_archive, 
                        dir[k].localizacao, 
                        dir[k].localizacao - dir[iguais].tamanho_disco, 
                        dir[k].tamanho_disco);

                        dir[k].localizacao -= dir[iguais].tamanho_disco;
                        dir[k].ordem --;
                }

                tam_trunc = tam_trunc + dir[iguais].tamanho_disco;
                for(int l = iguais; l < qtd_membros - 1; l++){
                    dir[l] = dir[l + 1];
                } 
                qtd_membros --; // deixar dps do laço (pq?)
            }
        }

        fseek(fp_archive, - tam_trunc, SEEK_END);
        long int trunc = ftell(fp_archive);
        ftruncate(fileno(fp_archive), trunc);
        
        for(int m = 0; m < qtd_membros; m++){
            fwrite(&dir[m], sizeof(struct membro), 1, fp_archive);
        }
        fwrite(&qtd_membros, sizeof(int), 1, fp_archive);
    }

    fclose(fp_archive);
}