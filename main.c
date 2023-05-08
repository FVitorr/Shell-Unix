#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> //fork()
#include <sys/types.h>
#include <sys/wait.h>

#define TAM 300
#define STG_PIPE "|"
#define STG_INPUT_FILE "<"
// cat -n < arquivo.txto comando irá exibir as linhas do arquivo arquivo.txt numeradas.
#define STG_OUTPUT_FILE ">"
// O comando echo "ola mundo" > arquivo.txt escreve a string "ola mundo" no arquivo de nome "arquivo.txt". Se o arquivo não existir, ele será criado. Se já existir, seu conteúdo será substituído pelo novo conteúdo.
#define PONTA_LEITURA 0
#define PONTA_ESCRITA 1

void lower(char *entry)
{
    int i = 0;
    while (i < TAM || entry[i] != '\n' || entry[i] != '\0')
    {
        entry[i] = tolower(entry[i]);
        if (entry[i] == '\n' || entry[i] == '\0')
        {
            return;
        }
        i++;
    }
    entry[i] = '\0';
}

void buscaARG(char *entry, char *args[], int *i)
{
    char *token = strtok(entry, " ");
    args[*i] = token;
    (*i)++;

    while (token != NULL)
    {
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            break;
        }
        args[*i] = token;
        // printf("args: %s\n",args[i]);
        (*i)++;
    }
    args[*i] = NULL;
}


int argsPipe(char *args[], int tam_arg, char *args_pipe[], int *tam_pipe, int init)
{
    int i;
    int t = 0;
    for (i = init; i <= tam_arg; i++)
    {
        if (args[i] != NULL && strcmp(args[i], "|") != 0)
        {
            args_pipe[*tam_pipe] = args[i];
            (*tam_pipe)++;
            t++;
        }
        else
        {
            args_pipe[*tam_pipe] = NULL;
            (*tam_pipe)++;
            return (i + 1);
        }
    }
    if (t == 0){//Retornar o outro argumento apos o pipe
        *tam_pipe = 0;
        for (i = init; i <= tam_arg; i++){
            args_pipe[*tam_pipe] = args[i];
            (*tam_pipe)++;
        }
        args_pipe[*tam_pipe] = NULL;
    }
    return -1;
}

int verificaPipe(char *args[], int tam_arg, int init)
{
    int qtd = 0;
    for (int i = init; i < tam_arg; i++)
    {
        if (strcmp(args[i], STG_PIPE) == 0)
        {
            qtd++;
        }
    }
    return qtd;
}

void criaPipe(int *pipefd)
{
    if (pipe(pipefd) == -1)
    { // Criar um pipe
        perror("Falha na criacao do pipe");
        exit(1);
    }
}

void readFile(FILE *fp, char namefile[])
{
    if ((fp = fopen(namefile, "r")) == NULL)
    {
        perror("Falha na Leitura");
        exit(1);
    }
    else
    {
        char entry[120];
        while (!feof(fp))
        {
            fgets(entry, 120, fp);
            printf("\n%s", entry);
        }
    }
}

void writeFile(FILE *fp, char namefile[], char text[120])
{
    if ((fp = fopen(namefile, "W")) == NULL)
    {
        printf("Erro ao criar arquivo");
    }
    else
    {
        fprintf(fp, "%s", text);
    }
}

pid_t do_fork(int pipefd[], int pontaPipe, int old_decriptor, char *args[])
{ // PIPE Direita descriptor = 1, old_decriptor = STDOUT_FILENO
    pid_t pid = fork();
    if (pid == -1)
    { /* fork() failed */
        perror("Erro no fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0){
     // BLOCO de exec para o filho
        printf("%d---- Processo Filho ----",pid);
        dup2(pipefd[pontaPipe], old_decriptor);

        close(pipefd[pontaPipe]); // libera as pontas de escrita e leitura do pipe
        if (pontaPipe == 1)
        {
            close(pipefd[0]);
        }
        else
        {
            close(pipefd[1]);
        }
        execvp(args[0], args); // substitui o binário do filho A pelo do programa apontado pelo progA
        perror("Falha na execvp");
        exit(1); // se o exec der errado, fecha o processo filho A pois não faz sentido continuar
    }            // filho A
    return pid;
}

void IOfile(char *args[], int tam_arg, int init)
{
    char namefile[120];
    char *args_[15];
    int tamArgs = 0;
    for (int i = init; i < tam_arg; i++)
    {
        if (strcmp(args[i], STG_OUTPUT_FILE) == 0)
        {                                  //> Salva no arquivo
            strcpy(namefile, args[i + 1]); // NameFILE

            FILE *fp = fopen(namefile, "r");
            int fp_ = fileno(fp); // descritor do arquivo
        }
        else if (strcmp(args[i], STG_INPUT_FILE) == 0)
        {                                  //< Alterar saida de dados para a escrita no arquivo
            strcpy(namefile, args[i + 1]); // NameFILE
            printf("{%s,%s,%s}", args_[0], args_[1], args_[2]);
        }
        else
        {
            args_[tamArgs++] = args[i];
        }
    }
}

int main()
{
    char entry[TAM];

    while (1)
    {
        printf("\n$ ");
        scanf("%[^\n]s", entry);
        setbuf(stdin, NULL);

        //strcpy(entry,"ls | grep main | grep c");

        lower(entry);

        if (strcmp(entry, "exit") == 0)
        {
            return 0; // Fechar processo pai
        }

        char *args[15];
        int tam_arg = 0;
        buscaARG(entry, args, &tam_arg);

        char *args_pipe[15];
        int tam_pipe = 0;
        int tpipe = verificaPipe(args, tam_arg, 0); // quantidade de pipes existente

        int init = 0; // Variavel de controle sobre a busca de argumentos para pipe(Guarda o INDEX do ultimo pipe + 1)
        int exec = 0;

        if (tpipe >= 1)
        { // TEM PIPE

            int pipefd[tpipe][2];
            // Criando pipes
            for(int i = 0; i < tpipe; i++) {
                criaPipe(pipefd[i]);
            }
            pid_t pidb;
            while (exec <= tpipe)
            {
                //printf("EXEC : %d  QTD PIPE: %d",exec,tpipe);
                init = argsPipe(args, tam_arg, args_pipe, &tam_pipe, init); // Encontrar Argumento
//                printf("\nCOMANDO: ");
//                for (int i = 0; i < tam_pipe;i++){
//                    printf(" %s ",args_pipe[i]);
//                }
                // ========== FILHO A ==========
                // Alterar o descriptor de SAIDA padrão (Monitor) para a ponta de LEITURA do PIPE - processo a direita
                if (exec == 0)
                { // primeira execute
                    //pidb = do_fork(pipefd[exec], PONTA_ESCRITA, STDOUT_FILENO, args_pipe);
                    pidb = do_fork(pipefd[exec], PONTA_ESCRITA, STDOUT_FILENO, args_pipe);
                    tam_pipe = 0;
                }
                else if (exec == tpipe)
                {
                    printf("\nTERCEIRO COMANDO: ");
                    for (int i = 0; i < tam_pipe;i++){
                        printf(" %s ",args_pipe[i]);
                    }
                    do_fork(pipefd[exec -1], PONTA_LEITURA, STDIN_FILENO, args_pipe);
                    // Fechando todas as pontas dos pipes
                    for (int j = 0; j < tpipe; j++)
                    {
                        close(pipefd[j][0]);
                        close(pipefd[j][1]);
                    }

                    // Aguardar o processo filho terminar
                    wait(NULL);
                    printf("\nFIM");
                }
                else{
                    printf("\nSEGUNDO COMANDO: ");
                    for (int i = 0; i < tam_pipe;i++){
                        printf(" %s ",args_pipe[i]);
                    }
                    pid_t pid = fork();
                    if(pid == -1) {
                        perror("fork");
                        exit(1);
                    }else if(pid == 0) {
                        printf("PID = %d",pid);
                        dup2(pipefd[exec -1][0], STDIN_FILENO);
                        dup2(pipefd[exec][1], STDOUT_FILENO);

                        execvp(args_pipe[0],args_pipe);
                        perror("execvp");
                        exit(1);
                    }
                    //waitpid(pid, NULL, 0);
                    tam_pipe = 0;
                } // intermediarios
                exec++; // pipes executado
                //wait(NULL);
                //printf("tpipe : %d\n", tpipe);
                //waitpid(pidb, NULL, 0); // Aguardar o processo filho terminar
                //ls | grep main | grep .c
            }

        }
        else
        {

            IOfile(args, tam_arg, 0);
            // Criar um filho com o fork
            pid_t pid = fork();
            if (pid == -1)
            { /* fork() failed */
                perror("Erro no fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0)
            {
                execvp(args[0], args); // executar comando basico
                perror("execvp");
                exit(0);
            }

            waitpid(pid, NULL, 0); // Aguardar o processo filho terminar
        }
    }
}
