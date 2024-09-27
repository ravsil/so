#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define RED "\033[1;31m\e[1m"
#define BLUE "\033[0;34m\e[1m"
#define GREEN "\033[0;32m\e[1m"
#define RESET "\033[0m"

void printDir(char *curDir, int waiting, int *child)
{
	int status;
	// wait for child to finish before printing
	if (waiting)
	{
		while ((*child = wait(&status)) > 0)
		{
			continue;
		}
	}
	printf(BLUE);
	printf("%s", getcwd(curDir, 255));
	printf(GREEN);
	printf("|>> ");
	printf(RESET);
}

void changeDir(char *cmd)
{
	if (chdir(cmd) == -1)
	{
		printf(RED);
		printf("Erro: caminho '%s' nao encontrado\n", cmd);
		printf(RESET);
	}
}

void getCommand(char *buffer, char *cmd, int *listen)
{
	if (*listen)
	{
		fgets(buffer, 255, stdin);
	}

	for (int i = 0; i < strlen(buffer); i++)
	{
		// copy buffer to cmd
		cmd[i] = buffer[i];
		if (buffer[i] == '&' && buffer[i + 1] == '&')
		{
			// stop cmd right before &
			cmd[i - 1] = '\0';
			*listen = 0;
			strcpy(buffer, buffer + i + 3);
			break;
		}
		if (buffer[i] == '\n')
		{
			cmd[i] = '\0';
			*listen = 1;
			break;
		}
	}
}

int main()
{
	// ignore ctrl+c
	signal(SIGINT, SIG_IGN);
	// starts as \n to print the current directory
	char buffer[255] = "\n";
	char cmd[255];
	char curDir[255];

	int id = -1;
	int listen = 0;

	printf("Digite 'ajuda' para ajuda\n");
	while (1)
	{
		// child id == 0
		if (id == 0)
		{
			system(cmd);
			break;
		}
		else
		{
			if (listen)
			{
				printDir(curDir, 1, &id);
			}

			getCommand(buffer, cmd, &listen);

			if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == ' ')
			{
				changeDir(cmd + 3);
			}
			else if (strcmp(cmd, "ajuda") == 0)
			{
				printf("Nao eh possivel navegar pelo texto, eh necessario apaga-lo\n");
			}
			else if (strcmp(cmd, "exit") == 0)
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
