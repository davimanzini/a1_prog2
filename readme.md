TRABALHO A1 DE PROGRAMAÇÃO 2 - "O ARQUIVOR VINAc"

Feito por Davi Chaves Rodrigues Dutra Manzini
GRR 20245482

Arquivos inclusos:

funcoes.c: Contém todas as funções utilizadas no trabalho.
    
    -mover() - Função auxiliar utilizada dentro de várias outras funções que é responsável por mover um bloco de bytes de uma posição do arquivo dado para outra.

    - insere() - Função responsável por tratar a inserção de arquivos no archive com uma flag que indica compressão do arquivo ou não. Membros já contidos no archive são substituidos. Novos membros são colocados no final do archive. (-ip e -ic)

    - lista_informacoes() - Função responsável para imprimir na tela o diretótio do archive, isto é, as informações de cada arquivo presente no archive. (-c)

    - remove_arquivos() - Função responsável por remover um ou mais arquivos dados de dentro do archive.(-r)

    - extrai_arquivos() - Função responsável pela extração de arquivos de dentro do archive. Dado um arquivo presente no archive, a função cria uma "cópia" com mesmo nome e conteúdo no diretório do trabalho. Se os membros a serem extraídos não forem indicados, a função extrai todos os membros. (-x)

    - move_arquivos() - Função responsável por mover arquivos dentro do archive para outra posição. A ordem dos arquivos dentro do archive é alterada. Caso não seja passado um target, o arquivo dado é movido para o começo do archive. (-m)

funcoes.h: Contém a struct (membro) utilizada para armazenar as informações de cada arquivo presente dentro do archive bem como os cabeçalhos das funções utilizadas no trabalho.

main.c: Função principal do trabalho que, a partir de um switch case e da manipulação de argc e argv[], direciona qual das funções listadas acima será executada de acordo com a chamada na linha de comando.

makefile: Arquivo make simples com a opção padrão make (gcc -Wall main.c funcoes.c lz.c -o vinac) e make clean (rm -f vinac).


Estrutura do archive:

Optei por construir o meu archive da seguinte forma: primeiro o conteúdo dos arquivos inseridos, depois um diretório com uma struct membro correspondente a cada arquivo presente com suas informações e, por fim, um inteiro guardando o número de arquivos no archive.