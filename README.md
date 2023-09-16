# Projeto Interpretador de Comandos (Shell) Unix

Este é um projeto que visa criar um interpretador de comandos, conhecido no jargão técnico Unix como **shell**. O objetivo principal é projetar e implementar um shell capaz de executar comandos, criar processos filhos e estabelecer comunicação entre eles usando pipes.

## Descrição do Projeto

O shell a ser desenvolvido terá as seguintes funcionalidades:

- Execução de comandos Unix padrão.
- Criação de processos filhos para executar comandos.
- Estabelecimento de comunicação entre os processos filhos e o processo pai usando pipes.
- Troca de dados entre os processos filhos e o shell.
- Fornecimento de dados de entrada para programas executados no shell.
- Captura de dados de saída de programas e exibição no shell.

## Funcionalidades Principais

### Execução de Comandos

O shell será capaz de executar comandos Unix padrão, permitindo ao usuário interagir com o sistema operacional e executar programas.

### Processos Filhos e Pipes

Processos filhos serão criados para executar comandos específicos. O uso de pipes permitirá a comunicação entre esses processos, tornando possível a troca de dados de entrada e saída entre eles e o shell.

### Troca de Dados

O shell facilitará a troca de dados entre os processos filhos e o processo pai. Isso inclui a capacidade de fornecer dados de entrada para programas e capturar dados de saída para exibição no shell.

## Como Usar

1. Clone este repositório em sua máquina local.

git clone https://github.com/seu-usuario/seu-repositorio.git

2. Compile o código-fonte do interpretador de comandos.

gcc -o shell shell.c

3. Execute o shell.

./shell