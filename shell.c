#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //fork()
#include <sys/wait.h>


#define STG_PIPE "|"
// cat -n < arquivo.txto comando irá exibir as linhas do arquivo arquivo.txt numeradas.
#define PONTA_LEITURA 0
#define PONTA_ESCRITA 1

int main(int argc, char** argv) {

    char entry[100]; //Entrada de dados
    char *args[30]; //Vetor q guarda os argumentos

    int pipe_indexArgs[30];//Armazena a posição do | no vetor de argumento

    while(1){
        printf("\n$ ");
        scanf("%[^\n]s",entry); // scanf() guarda a linha toda na string entry
        setbuf(stdin, NULL);
        printf("\n");

        int tam_args = 0;
        int qtdPipe = 1;

        if(strcmp(entry, "exit") == 0){
            break;
        }

        char *token = strtok(entry," "); // tokeniza a entrada usando espaço como separador
        pipe_indexArgs[0] = 0; //

        while(token != NULL){
            if(strcmp(token,STG_PIPE) != 0){ //Não encontou o pipe
                //args_pipe[tam_pipe] = tam_args+1;
                args[tam_args] = token; // o pipe precisa ser null para o execvp() saber onde parar de executar o comando

            }else{ //
                pipe_indexArgs[qtdPipe] = tam_args + 1; // posição para inicio do arg após o pipe
                args[tam_args] = NULL; // substitiu pipe por NULL
                qtdPipe++;

            }
            tam_args++; //próximo token
            token = strtok(NULL," "); // pega o próximo token da string 'entry'
        }
        args[tam_args] = NULL; //atribuir

        int pipefd[qtdPipe][2]; // descritor de arquivos

        for(int i = 0;i < qtdPipe; i++){
            if (pipe(pipefd[i]) == -1){ // Criar um pipe
                perror("Falha na criacao do pipe: ");
                exit(1);
            }
        }
        int exec = 0;
        pid_t pid;
        FILE *y;
        while(exec < qtdPipe) {

            pid = fork();
            if (pid == 0) {

                int indice = pipe_indexArgs[exec];
                // indice_arq copia do indice mas usado para quando se tem > ou <
                int indice_arq = indice;

                while(args[indice_arq] != NULL){
                    // verifica se existe redirecionamento de entrada
                    if(strcmp(args[indice_arq], ">") == 0){
                        y = fopen(args[indice_arq+1], "w");
                        dup2(fileno(y), STDOUT_FILENO);
                        close(fileno(y));
                        fclose(y);
                        args[indice_arq] = NULL;
                        // verifica se existe redirecionamento de saida
                    }else if(strcmp(args[indice_arq], "<") == 0){
                        y = fopen(args[indice_arq+1], "r");
                        dup2(fileno(y), STDIN_FILENO);
                        args[indice_arq] = NULL;
                    }
                    indice_arq++;
                }

                if (exec != 0) { // caso nao seja o primeiro processo
                    //printf("\n[%d]Processo - Primeiro IF - dup2(pipefd[%d][PONTA_LEITURA], STDIN_FILENO)", exec,exec-1);
                    close(pipefd[exec-1][PONTA_ESCRITA]); // fecha o descritor de escrita
                    dup2(pipefd[exec-1][PONTA_LEITURA], STDIN_FILENO); // muda o descritor de leitura para stdin
                    close(pipefd[exec-1][PONTA_LEITURA]); // fecha o descritor de leitura

                }

                if (exec != qtdPipe - 1) { // caso nao seja o ultimo processo (primeira exec bate aqui)
                    //printf("\n[%d]Processo - Segundo IF - dup2(pipefd[%d][PONTA_ESCRITA], STDOUT_FILENO)", exec,exec);
                    close(pipefd[exec][PONTA_LEITURA]); // fecha o descritor de leitura
                    dup2(pipefd[exec][PONTA_ESCRITA], STDOUT_FILENO); // muda o descritor de escrita para stdout
                    close(pipefd[exec][PONTA_ESCRITA]); // fecha o descritor de escrita
                }

                // executa comandos
                execvp(args[indice],&args[indice]);
                close(pipefd[exec-1][0]);
                exit(0);
            } else if (pid > 0) {
                //close(y);
                close(pipefd[exec - 1][0]);
                close(pipefd[exec - 1][1]);

            }else {
                perror("fork");
                exit(1);
            }
            exec++;
        }
        waitpid(pid,NULL,0);
    }


    return (EXIT_SUCCESS);
}