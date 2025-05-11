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


void insere(char *archive, char *arquivo, int comprimir){ //n é numero de arquivos

    FILE *arquivador = fopen(archive, "rb+"); //qqr mode com r só funciona se o arquivo ja existir
    if(!arquivador){
        arquivador = fopen(archive, "wb+"); //nao podemos usar só wb pq esse mode apaga tudo do arquivo
        if(!arquivador){
            perror("Erro ao criar o archive");
            return;
        }
    }

    fseek(arquivador, 0, SEEK_END);
    long int tamanho_total = ftell(arquivador);

    int qtd_membros = 0;

    if(tamanho_total <= sizeof(int)){ //archive vazio!!

        struct membro diretorio[1];

        FILE *fp_membro = fopen(arquivo, "rb");
        if(!fp_membro){
            perror("Erro ao a abrir membro");
        }

        fseek(fp_membro, 0, SEEK_END);
        long int tamanho = ftell(fp_membro);
        fseek(fp_membro, 0, SEEK_SET); //volta o ponteiro para o inicio do arquivo
        
        char *buffer = malloc(tamanho); //reserva espaço temp. na RAM
        if(!buffer){
            perror("Erro ao abrir o arquivo");
            fclose(fp_membro);
        }
        
        fread(buffer, 1, tamanho, fp_membro); 
        long int posicao = ftell(arquivador); 
        fwrite(buffer, 1, tamanho, arquivador); 
        fclose(fp_membro);
        free(buffer);

        struct membro novo_membro;

        strcpy(novo_membro.nome, arquivo);
        novo_membro.data_mod         = time(NULL);
        novo_membro.ordem            = 0;
        novo_membro.uid              = getuid();
        novo_membro.tamanho_original = tamanho;
        novo_membro.tamanho_disco    = tamanho;
        novo_membro.localizacao      = posicao;

        diretorio[0] = novo_membro;

        int qtd_membros = 1;

        fseek(arquivador, 0, SEEK_END);
        fwrite(&diretorio[0], sizeof(struct membro), 1, arquivador);
    
        fwrite(&qtd_membros, sizeof(int), 1, arquivador); 

        fclose(arquivador);
    }

    else{ //archive nao esta vazio

        fseek(arquivador, - sizeof(int), SEEK_END); //cm nao esta vazio, o ultimo item eh um int tamanho
        fread(&qtd_membros, sizeof(int), 1, arquivador); //guarda qnts arquivos tem
        fseek(arquivador, - sizeof(int) - (qtd_membros * sizeof(struct membro)), SEEK_END); //coloca ptr no cmc do diretorio


        struct membro dir[qtd_membros]; 

        for(int i = 0; i < qtd_membros; i ++){ //colocamos os membros do archive dentro de um vetor de membros (dir)
            fread(&dir[i], sizeof(struct membro), 1, arquivador);
        }

        int iguais = -1;

        for(int i = 0; i < qtd_membros; i++){
            if(strcmp(arquivo, dir[i].nome) == 0){ //achou um arquivo no archive com nome igual
                iguais = i;
                break; //sai do for de dentro 
            }
            
        }

        if(iguais != -1){ //encontrou o arquivo no archive

            FILE *fp_novo = fopen(arquivo, "rb");
            if(!fp_novo){
                perror("Erro ao abrir o arquivo novo");
            }

            fseek(fp_novo, 0, SEEK_END);
            long int tam_insert = ftell(fp_novo); //tamanho do arq a ser inserido
            fseek(fp_novo, 0, SEEK_SET);
            
            if(tam_insert == dir[iguais].tamanho_disco){ //tamanhos iguais

                char *buffer = malloc(tam_insert);

                fread(buffer, 1, tam_insert, fp_novo);
                fclose(fp_novo);

                fseek(arquivador, dir[iguais].localizacao, SEEK_SET);
                fwrite(buffer, 1, tam_insert, arquivador);
                free(buffer);

                dir[iguais].data_mod = time(NULL); //atualiza infos membro
                dir[iguais].uid      = getuid();

                fseek(arquivador, - sizeof(int) - 
                (qtd_membros * (sizeof(struct membro))), 
                SEEK_END);
            
                for(int k = 0; k < qtd_membros; k++){
                    fwrite(&dir[k], sizeof(struct membro), 1, arquivador);
                }
            }

            else if(tam_insert > dir[iguais].tamanho_disco){ //tamanho novo maior

                long int tam_diff = tam_insert - dir[iguais].tamanho_disco;
            
                fseek(arquivador, - sizeof(int) - (qtd_membros * sizeof(struct membro)), SEEK_END);
                long int pos_trunc = ftell(arquivador);
            
                ftruncate(fileno(arquivador), pos_trunc);

                for(int l = qtd_membros - 1; l > iguais; l--){ //movendo arquivos
            
                        mover(arquivador,
                            dir[l].localizacao,
                            dir[l].localizacao + tam_diff,
                            dir[l].tamanho_disco);
                        
                            dir[l].localizacao += tam_diff;
                }
            
                char *buffer = malloc(tam_insert);
                if(!buffer){
                    perror("Erro ao alocando buffer");
                    fclose(fp_novo);
                }
            
                fread(buffer, 1, tam_insert, fp_novo);
                fclose(fp_novo);
            
                fseek(arquivador, dir[iguais].localizacao, SEEK_SET);
                fwrite(buffer, 1, tam_insert, arquivador);
                free(buffer);
            
                dir[iguais].tamanho_disco    = tam_insert;
                dir[iguais].tamanho_original = tam_insert;
                dir[iguais].data_mod         = time(NULL);
                dir[iguais].uid              = getuid();
            
                fseek(arquivador, 0, SEEK_END);
            
                for (int m = 0; m < qtd_membros; m++) {
                    fwrite(&dir[m], sizeof(struct membro), 1, arquivador);
                }
            
                fwrite(&qtd_membros, sizeof(int), 1, arquivador);
            }

            
            else if(tam_insert < dir[iguais].tamanho_disco){ //tamanho antigo maior

                long int tam_diff = dir[iguais].tamanho_disco - tam_insert;

                for(int k = qtd_membros - 1; k >= 0; k--){
                    if(dir[k].localizacao > dir[iguais].localizacao){
                        mover(arquivador, dir[k].localizacao, dir[k].localizacao - tam_diff, dir[k].tamanho_disco);
                        dir[k].localizacao -= tam_diff;
                    }
                }

                char *buffer = malloc(tam_insert);
                if(!buffer){
                    perror("Erro ao alocando buffer");
                    fclose(fp_novo);
                }

                fread(buffer, 1, tam_insert, fp_novo);
                fclose(fp_novo);

                fseek(arquivador, dir[iguais].localizacao, SEEK_SET);
                fwrite(buffer, 1, tam_insert, arquivador);
                free(buffer);

                dir[iguais].tamanho_disco    = tam_insert;
                dir[iguais].tamanho_original = tam_insert;
                dir[iguais].data_mod         = time(NULL);
                dir[iguais].uid              = getuid();

                fseek(arquivador, - 
                    sizeof(int) - 
                    (qtd_membros * sizeof(struct membro)) - 
                    tam_diff, 
                    SEEK_END);
                
                long int pos_trunc = ftell(arquivador);
                
                ftruncate(fileno(arquivador), pos_trunc);

                fseek(arquivador, 0, SEEK_END);

                for (int m = 0; m < qtd_membros; m++) {
                    fwrite(&dir[m], sizeof(struct membro), 1, arquivador);
                }

                fwrite(&qtd_membros, sizeof(int), 1, arquivador);
            }
        }

        else{ //nao encontrou o arquivo no archive

            FILE *novo_arquivo = fopen(arquivo, "rb+");
            if(!novo_arquivo){
                perror("Erro ao abrir novo arquivo");
                fclose(arquivador);
            }
            //nao pode fazer isso, aqui esta o erro
            struct membro *aux = malloc((qtd_membros + 1) * sizeof(struct membro));

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
                fclose(arquivador);
            }

            fread(buffer, 1, tamanho, novo_arquivo);
            fclose(novo_arquivo);

            fseek(arquivador, 0, SEEK_END); 

            long int pos_insercao = ftell(arquivador) - sizeof(int) - (qtd_membros * sizeof(struct membro));

            ftruncate(fileno(arquivador), pos_insercao); //truncamos o arquivo, tiramos diretorio e int

            fseek(arquivador, 0, SEEK_END);
            fwrite(buffer, tamanho, 1, arquivador); //escreve novo arq no archive

            free(buffer);

            struct membro novo;

            strcpy(novo.nome, arquivo);
            novo.data_mod         = time(NULL);
            novo.ordem            = qtd_membros;
            novo.uid              = getuid();
            novo.tamanho_original = tamanho;
            novo.tamanho_disco    = tamanho;
            novo.localizacao      = pos_insercao;

            aux[qtd_membros] = novo; //checar
            qtd_membros ++;

            for(int l = 0; l < qtd_membros; l++){
                fwrite(&aux[l], sizeof(struct membro), 1, arquivador);
            }

            fwrite(&qtd_membros, sizeof(int), 1, arquivador);
            free(aux);
        }
    }
    fclose(arquivador);
}
//separacao de funcoes


void lista_informacoes(char *archive){ // -c

    FILE *arquivador = fopen(archive, "rb");
    if(!arquivador){
        perror("Erro ao abrir o archive");
        return;
    }

    fseek(arquivador, 0, SEEK_END);
    long int tamanho = ftell(arquivador);

    if(tamanho <= sizeof(int)){
        printf("Archive está vazio.\n");
        fclose(arquivador);
        return;
    }

    int qtd_arquivos = 0;

    fseek(arquivador, -sizeof(int), SEEK_END);
    if(fread(&qtd_arquivos, sizeof(int), 1, arquivador) != 1){
        perror("Erro ao ler qtd_arquivos");
        fclose(arquivador);
        return;
    }

    printf("Quantidade de arquivos no archive: %d\n\n", qtd_arquivos);

    fseek(arquivador, -sizeof(int) - (qtd_arquivos * sizeof(struct membro)), SEEK_END);

    for(int i = 0; i < qtd_arquivos; i++){
        struct membro dir;
        if(fread(&dir, sizeof(struct membro), 1, arquivador) != 1){  //precisa disso?
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

    fclose(arquivador);
}


void remove_arquivos(char *archive, char **arquivos, int n){

    FILE *arquivador = fopen(archive, "rb+");
    if(!arquivador){
        perror("Erro ao abrir o archive");
        return;
    }

    fseek(arquivador, 0, SEEK_END);
    long int tam_archive = ftell(arquivador);

    if (tam_archive <= sizeof(int)) { //archive vazio
        printf("Erro: o archive está vazio!\n");
        fclose(arquivador);
        return;
    }

    else{ //archive nao esta vazio

        int qtd_membros;
        fseek(arquivador, - sizeof(int), SEEK_END);
        fread(&qtd_membros, sizeof(int), 1, arquivador);
        
        fseek(arquivador, - 
            sizeof(int) - 
            (qtd_membros * sizeof(struct membro)), 
            SEEK_END);

        struct membro dir[qtd_membros];

        for(int i = 0; i < qtd_membros; i ++){ //colocamos os membros do archive dentro de um vetor de membros (dir)
            fread(&dir[i], sizeof(struct membro), 1, arquivador);
        }

        fseek(arquivador, 
            - sizeof(int) 
            - (qtd_membros * sizeof(struct membro)), 
            SEEK_END);

        long int pos_trunc = ftell(arquivador); //truncar soh no final?

        ftruncate(fileno(arquivador), pos_trunc); //trunca o diretorio e o count

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
                    mover(arquivador, 
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
                qtd_membros --;
            }
        }

        fseek(arquivador, - tam_trunc, SEEK_END);
        long int trunc = ftell(arquivador);
        ftruncate(fileno(arquivador), trunc);

        for(int m = 0; m < qtd_membros; m++){
            fwrite(&dir[m], sizeof(struct membro), 1, arquivador);
        }
        fwrite(&qtd_membros, sizeof(int), 1, arquivador);
    }

    fclose(arquivador);
}

void extrai_arquivos(char *archive, char **arquivos, int n){

    FILE *arquivador = fopen(archive, "rb+");
    if(!arquivador){
        perror("Erro ao abrir o archive");
        return;
    }

    fseek(arquivador, 0, SEEK_END);
    long int tam = ftell(arquivador);

    if(tam <= sizeof(int)){ //archive vazio
        printf("Erro: archive vazio");
        fclose(arquivador);
        return;
    }

    else{ //archive nao esta vazio

        int qtd_membros;
        fseek(arquivador, - sizeof(int), SEEK_END);
        fread(&qtd_membros, sizeof(int), 1, arquivador);

        struct membro dir[qtd_membros];

        fseek(arquivador, - 
            sizeof(int) - 
            (qtd_membros * sizeof(struct membro)), 
            SEEK_END);

        for(int i = 0; i < qtd_membros; i ++){ 
            fread(&dir[i], sizeof(struct membro), 1, arquivador);
        }

        if(n == 0){ //extrair todos os arquivos de archive

            for(int i = 0; i < qtd_membros; i++){

                FILE *arquivo_x = fopen(dir[i].nome, "wb"); //checar
                    if(!arquivo_x){
                        perror("Erro ao criar arquivo de extracao!");
                        continue;
                    }
                    if(0 > 3){ //aqui testar se esta comprimido e descomprimir

                    }

                    else{ //nao esta comprimido

                        fseek(arquivador, dir[i].localizacao, SEEK_SET);
                        
                        char *buffer = malloc(dir[i].tamanho_disco); //checar
                        fread(buffer, dir[i].tamanho_disco, 1, arquivador);
                        fwrite(buffer, dir[i].tamanho_disco, 1, arquivo_x);
                        free(buffer);
                    }
                    fclose(arquivo_x);
            }

        }

        else{ //extrair apenas os arquivos solicitados

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

                    FILE *arquivo_x = fopen(dir[iguais].nome, "wb"); //checar
                    if(!arquivo_x){
                        perror("Erro ao criar arquivo de extracao!");
                        continue;
                    }
                    if(0 > 3){ //aqui testar se esta comprimido e descomprimir

                    }

                    else{ //nao esta comprimido

                        fseek(arquivador, dir[iguais].localizacao, SEEK_SET);
                        
                        char *buffer = malloc(dir[iguais].tamanho_disco); //checar
                        fread(buffer, dir[iguais].tamanho_disco, 1, arquivador);
                        fwrite(buffer, dir[iguais].tamanho_disco, 1, arquivo_x);
                        free(buffer);
                    }
                    fclose(arquivo_x);
                } 
            }   
        }

    }
    fclose(arquivador);
}


void move_arquivos(char *archive, char **arquivos, int n){


    FILE *arquivador = fopen(archive, "rb+");
    if(!arquivador){
        perror("Erro ao abrir o archive");
        return;
    }
 
 
    fseek(arquivador, 0, SEEK_END);
    long int tam = ftell(arquivador);
 
 
    if(tam <= sizeof(int)){
        printf("Archive vazio, nao ha o que mover!");
        fclose(arquivador);
        return;
    }
 
 
    else{ //archive nao esta vazio
        
        // checar se há target ou se é null!

        int qtd_membros;

        fseek(arquivador, - sizeof(int), SEEK_END);
        fread(&qtd_membros, sizeof(int), 1, arquivador);

        struct membro dir[qtd_membros];

        fseek(arquivador, - sizeof(int) - (qtd_membros * sizeof(struct membro)), SEEK_END);

        for(int i = 0; i < qtd_membros; i++){
            fread(&dir[i], sizeof(struct membro), 1, arquivador);
        }
 
        
        if(n == 1){ //mover para o inicio
            
            int indice_mover = -1;
            for(int i = 0; i < qtd_membros; i++){
                if(strcmp(arquivos[0], dir[i].nome) == 0){
                    indice_mover = i;
                }
            }

            if(indice_mover == -1){
                printf("Erro: arquivo a ser movido nao esta no archive! \n");
                fclose(arquivador);
                return;
            }

            if(strcmp(dir[indice_mover].nome, dir[0].nome) == 0){
                printf("Erro: arquivo a ser movido ja esta no inicio do archive! \n");
                fclose(arquivador);
                return;
            }

            long int tam_mover = dir[indice_mover].tamanho_disco;

            //trunca diretorio e count
            fseek(arquivador, - sizeof(int) - (qtd_membros * sizeof(struct membro)), SEEK_END);
            long int pos_trunc = ftell(arquivador);
            ftruncate(fileno(arquivador), pos_trunc);

            //le arquivo mover para o buffer
            char *buffer = malloc(dir[indice_mover].tamanho_disco);
            fseek(arquivador, dir[indice_mover].localizacao, SEEK_SET);
            fread(buffer, tam_mover, 1, arquivador);

            fseek(arquivador, 0, SEEK_END);
            fwrite(buffer, tam_mover, 1, arquivador);
            free(buffer);

            for(int i = indice_mover - 1; i > -1; i--){

                mover(arquivador, dir[i].localizacao,
                     dir[i].localizacao + tam_mover, dir[i].tamanho_disco);

                dir[i].localizacao += tam_mover;
                dir[i].ordem ++;
            }

            //coloca o mover no comeco do arquivo
            char *buffer_fix = malloc(tam_mover);
            fseek(arquivador, - tam_mover, SEEK_END);
            fread(buffer, tam_mover, 1, arquivador);
            fseek(arquivador, 0, SEEK_SET);
            fwrite(buffer, tam_mover, 1, arquivador);
            free(buffer_fix);

            dir[indice_mover].localizacao = 0;
            dir[indice_mover].ordem = 0;

            fseek(arquivador, - tam_mover, SEEK_END);
            long int trunc_final = ftell(arquivador);

            ftruncate(fileno(arquivador), trunc_final);
            
            struct membro aux = dir[indice_mover];

            for(int i = indice_mover; i > 0; i--){
                dir[i] = dir[i - 1];
            }

            dir[0] = aux;

            for(int m = 0; m < qtd_membros; m++){
                fwrite(&dir[m], sizeof(struct membro), 1, arquivador);
            }
        
            fwrite(&qtd_membros, sizeof(int), 1, arquivador);

        }

        else{
            int indice_mover = -1;
            int indice_target = -1;


            for(int i = 0; i < qtd_membros; i++){

                if(strcmp(arquivos[0], dir[i].nome) == 0){ //arquivo a ser movido
                    indice_mover = i;
                }
                if(strcmp(arquivos[1], dir[i].nome) == 0){
                    indice_target = i;
                }
            }
        
            if(indice_mover == -1  || indice_target == -1){
                printf("Erro: arquivo a ser movido ou arquivo target nao estao no archive!\n");
                fclose(arquivador);
                return;
            }
    
            if(indice_mover == indice_target){ //caso membro a ser movido e target sejam iguais
                printf("Erro: indices iguais!\n");
                fclose(arquivador);
                return;
            }
    
            //CASOS POSSIVIES:

            //primeiro truncamos diretorio e int para termos só os arquivos no archive
            fseek(arquivador, - sizeof(int) - (qtd_membros * sizeof(struct membro)), SEEK_END);
            long int pos_trunc = ftell(arquivador);
            ftruncate(fileno(arquivador), pos_trunc);

            long int tam_mover = dir[indice_mover].tamanho_disco; //disco ou original?? checar se ta comprimido?
            long int tam_target = dir[indice_target].tamanho_disco; //mesma duvida

            if(indice_mover < indice_target){ //mover pra depois

                //escreve arquivo a mover no final do archive
                char *buffer = malloc(tam_mover);
                fseek(arquivador, dir[indice_mover].localizacao, SEEK_SET);
                fread(buffer, tam_mover, 1, arquivador);
                fseek(arquivador, 0, SEEK_END);
                fwrite(buffer, tam_mover, 1, arquivador);
                free(buffer);

                for(int i = indice_mover + 1; i <= indice_target; i++){

                    mover(arquivador, dir[i].localizacao,
                        dir[i].localizacao - tam_mover, dir[i].tamanho_disco);

                    dir[i].localizacao -= tam_mover;
                    dir[i].ordem --;
                }

                //coloca mover no buffer
                char *buffer_fix = malloc(tam_mover);
                fseek(arquivador, - tam_mover, SEEK_END);
                fread(buffer_fix, tam_mover, 1, arquivador);

                //ajusta ponteiro para o final do target
                fseek(arquivador, dir[indice_target].localizacao + tam_target,
                    SEEK_SET);
                
                fwrite(buffer_fix, tam_mover, 1, arquivador); //coloca mover no lugar certo
                free(buffer_fix);

                dir[indice_mover].localizacao = dir[indice_target].localizacao + tam_target;
                dir[indice_mover].ordem = dir[indice_target].ordem + 1;

                fseek(arquivador, - tam_mover, SEEK_END);
                long int trunc_final = ftell(arquivador);

                ftruncate(fileno(arquivador), trunc_final);
                
                struct membro aux = dir[indice_mover];

                for(int i = indice_mover; i < indice_target; i++){
                    dir[i] = dir[i + 1];
                }

                dir[indice_target] = aux;

                for(int m = 0; m < qtd_membros; m++){
                    fwrite(&dir[m], sizeof(struct membro), 1, arquivador);
                }
            
                fwrite(&qtd_membros, sizeof(int), 1, arquivador);
            }

            if(indice_mover > indice_target){ //mover pra antes

                //escreve arquivo a mover no final do archive
                char *buffer = malloc(tam_mover);
                fseek(arquivador, dir[indice_mover].localizacao, SEEK_SET);
                fread(buffer, tam_mover, 1, arquivador);
                fseek(arquivador, 0, SEEK_END);
                fwrite(buffer, tam_mover, 1, arquivador);
                free(buffer);

                for(int i = indice_mover - 1; i > indice_target; i--){
                    mover(arquivador, dir[i].localizacao,
                        dir[i].localizacao + tam_mover, dir[i].tamanho_disco);

                    dir[i].localizacao += tam_mover;
                    dir[i].ordem ++;
                }

                //coloca mover no buffer
                char *buffer_fix = malloc(tam_mover);
                fseek(arquivador, - tam_mover, SEEK_END);
                fread(buffer_fix, tam_mover, 1, arquivador);

                fseek(arquivador, dir[indice_target].localizacao + tam_target,
                    SEEK_SET);

                fwrite(buffer_fix, tam_mover, 1, arquivador); //coloca mover no lugar certo
                free(buffer_fix);

                dir[indice_mover].localizacao = dir[indice_target].localizacao + tam_target;
                dir[indice_mover].ordem = dir[indice_target].ordem + 1;

                fseek(arquivador, - tam_mover, SEEK_END);
                long int trunc_final = ftell(arquivador);

                ftruncate(fileno(arquivador), trunc_final);

                struct membro aux = dir[indice_mover];

                for(int i = indice_mover; i > indice_target + 1; i--){
                    dir[i] = dir[i - 1];
                }

                dir[indice_target + 1] = aux;

                for(int i = 0; i < qtd_membros; i++){
                    fwrite(&dir[i], sizeof(struct membro), 1, arquivador);
                }
            
                fwrite(&qtd_membros, sizeof(int), 1, arquivador);
            }
        }
        fclose(arquivador);
    }
 }
 