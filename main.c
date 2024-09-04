#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define RED "\033[1;31m\e[1m"
#define BLUE "\033[0;34m\e[1m"
#define GREEN "\033[0;32m\e[1m"
#define RESET "\033[0m"

void printDir(char *curDir)
{
	printf(BLUE);
	printf("%s", getcwd(curDir, 255));
	printf(GREEN);
	printf("|>> ");
	printf(RESET);
}

void changeDir(char *buffer, char *curDir)
{
	if (chdir(buffer) == -1)
	{
		printf(RED);
		printf("Erro: caminho '%s' nao encontrado\n", buffer);
		printf(RESET);
	}
	printDir(curDir);
}

void help(char *curDir)
{
	printf("O comando cd nao pode ser executado juntamente de nenhum outro\n");
	printf("caso1: echo hello && cd ..\n");
	printf("'cd ..' nao sera executado\n");
	printf("caso2: cd .. && echo hello\n");
	printf("ambos os comandos irao falhar\n\n");
	printf("Alguns programas como vi nao funcionam como deveriam\n\n");
	printDir(curDir);
}

int main()
{
	// ignore ctrl+c
	signal(SIGINT, SIG_IGN);

	char buffer[255];
	char curDir[255];
	int id = -1;

	printf("Digite 'ajuda' para ajuda\n");
	printDir(curDir);
	while (1)
	{
		// child id == 0
		if (id == 0)
		{
			system(buffer);
			printDir(curDir);
			break;
		}
		else
		{
			fgets(buffer, 255, stdin);
			// remove \n
			buffer[strlen(buffer) - 1] = '\0';
			if (buffer[0] == 'c' && buffer[1] == 'd' && buffer[2] == ' ')
			{
				changeDir(buffer + 3, curDir);
			}
			else if (strcmp(buffer, "ajuda") == 0)
			{
				help(curDir);
			}
			else if (strcmp(buffer, "exit") == 0)
			{
				break;
			}
			else
			{
				id = fork();
			}
		}
	}
	return 0;
}
