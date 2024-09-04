#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int changeDir(char *buffer, char *curDir)
{
	if (chdir(buffer) == -1)
	{
		printf("Erro: caminho '%s' nao encontrado\n", buffer);
	}
	else
	{
		printf("%s|>> ", getcwd(curDir, 255));
	}
}

void problems(char *curDir)
{
	printf("Se o comando 'cd' for executado juntamente de outro comando, o segundo argumento nao funcionara\n");
	printf("caso1: echo hello && cd ..\n");
	printf("'cd ..' nao sera executado\n");
	printf("caso2: cd .. && echo hello\n");
	printf("'echo hello' nao sera executado\n\n");
	printf("Se um programa como python for executado, o input do teclado sera extremamente prejudicado\n\n");
	printf("Se um comando que eh executado constantemente (como 'ping') for acionado, ao parar sua execucao o terminal tambem sera encerrado\n");
	printf("%s|>> ", getcwd(curDir, 255));
}

int main()
{
	printf("Terminal\n");
	char buffer[255];
	char curDir[255];
	int id = -1;
	printf("Digite 'problemas' para saber os problemas conhecidos\n");
	printf("%s|>> ", getcwd(curDir, 255));
	while (1)
	{
		// child id == 0
		if (id == 0)
		{
			system(buffer);
			printf("%s|>> ", getcwd(curDir, 255));
			break;
		}
		else
		{
			fgets(buffer, 255, stdin);
			buffer[strlen(buffer) - 1] = '\0';
			if (buffer[0] == 'c' && buffer[1] == 'd' && buffer[2] == ' ')
			{
				changeDir(buffer + 3, curDir);
			}
			else if (strcmp(buffer, "problemas") == 0)
			{
				problems(curDir);
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