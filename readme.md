# TRABALHO A1 DE PROGRAMAÇÃO 2 - "O ARQUIVOR VINAc"

Feito por Davi Chaves Rodrigues Dutra Manzini  
GRR 20245482

## Arquivos inclusos:

**funcoes.c**: Contém todas as funções utilizadas no trabalho.
- mover() - Função auxiliar utilizada dentro de várias outras funções que é responsável por mover um bloco de bytes de uma posição do arquivo dado para outra.
- insere() - Função responsável por tratar a inserção de arquivos no archive com uma flag que indica compressão do arquivo ou não. Membros já contidos no archive são substituidos. Novos membros são colocados no final do archive. (-ip e -ic)
- lista_informacoes() - Função responsável para imprimir na tela o diretótio do archive, isto é, as informações de cada arquivo presente no archive. (-c)
- remove_arquivos() - Função responsável por remover um ou mais arquivos dados de dentro do archive.(-r)
- extrai_arquivos() - Função responsável pela extração de arquivos de dentro do archive. Dado um arquivo presente no archive, a função cria uma "cópia" com mesmo nome e conteúdo no diretório do trabalho. Se os membros a serem extraídos não forem indicados, a função extrai todos os membros. (-x)
- move_arquivos() - Função responsável por mover arquivos dentro do archive para outra posição. A ordem dos arquivos dentro do archive é alterada. Caso não seja passado um target, o arquivo dado é movido para o começo do archive. (-m)

**funcoes.h**: Contém a struct (membro) utilizada para armazenar as informações de cada arquivo presente dentro do archive bem como os cabeçalhos das funções utilizadas no trabalho.

**main.c**: Função principal do trabalho que, a partir de um switch case e da manipulação de argc e argv[], direciona qual das funções listadas acima será executada de acordo com a chamada na linha de comando.

**makefile**: Arquivo make simples com a opção padrão make (gcc -Wall main.c funcoes.c lz.c -o vinac) e make clean (rm -f vinac).

**readme.md**: O presente arquivo, o qual contém informações sobre cada arquivo utilizado, uma descrição breve sobre cada função utilizada e um relatório sobre o processo geral de desenvolvimento do trabalho.

---

## Estrutura do archive:

Optei por construir o meu archive da seguinte forma: primeiro o conteúdo dos arquivos inseridos, depois um diretório com uma struct membro correspondente a cada arquivo presente com suas informações e, por fim, um inteiro guardando o número de arquivos no archive. Guardar a quantidade de arquivos presentes no final do archive foi essencial para mim durante o desenvolvimento do trabalho, de modo que esse valor foi usado como parâmetro para várias das funções.

---

## Processo de desenvolvimento e dificuldades encontradas:

Sem dúvidas a parte mais díficil do trabalho inteiro foi começar. Tive bastante dificuldade em entender direito o que eu teria que fazer e como eu poderia começar. Comecei tirando dúvidas com colegas e com o professor em sala sobre o archive e como deveria ser sua estrutura. Em seguida, utilizei os powerpoints disponíveis no Moodle junto com o site do Dr. Professor Carlos Maziero para estudar mais profundamente como funciona a manipulação de arquivos em C: estudei a utilização das funções mais comuns como fwrite, fread, fseek, ftell, etc e como poderia usar elas no contexto do arquivador VINAc.

A partir disso, decidi começar pela função de inserção, uma vez que todas as outras funções dependiam de haver algo já inserido no archive. Nessa função, assim como nas seguintes, decidi primeiro pensar nos casos possíveis e montar um esqueleto principal e depois realizar a implementação propriamente dita de cada caso. Com o tempo me habituei às funções e à lógica geral que deveria utilizar para o resto do trabalho e consegui aumentar bastante a velocidade com a qual escrevia as funções.

Conforme escrevia cada função, eu realizava testes abrangendo os diferentes casos possíveis para garantir que aquela parte do código estava correta. Só quando tinha garantia que a implementação da função atual estava correta eu passava para a escrita da próxima.

De modo geral, o desenvolvimento do trabalho depois do começo mais complicado fluiu de forma muito boa. A única próxima grande dificuldade que tive depois foi na resolução de um bug na função de inserção que até então não tinha notado. Na inserção de um arquivo que já estava presente no archive juntamente com pelo menos mais um, a impressão do diretório e o conteúdo do archive ficavam errados. Tentei por vários dias resolver mas, como minha função é muito grande, estava com dificuldade de encontrar a origem do problema. Por recomendação do Dr. Professor David Menotti, resolvi reescrever a função inteira e foi assim que consegui enontrar o problema: em certo ponto da função eu estava tentando alterar o tamanho de um vetor estático. Após resolver esse erro e alterar minimamente a lógica geral da minha função, os testes funcionaram!

Em conclusão, esse definitavamente foi um trabalho com qual aprendi muito. Estou extremamente satisfeito com o resultado e por ter praticado tanto quanto pratiquei. Foi um pocesso muito enriquecedor para o meu aprendizado no curso.