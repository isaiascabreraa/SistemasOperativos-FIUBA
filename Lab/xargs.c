
#ifndef NARGS
#define NARGS 4
#endif

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

const int ERROR = -1;

// Pre: -
// Post: Sustituye el proceso actual por el enviado desde la linea de comandos.
void
imprimir_argumentos(char *argumentos[NARGS + 2], int *contador_argumentos)
{
	int res;
	argumentos[*contador_argumentos + 1] =
	        NULL;  // NULL para finalizar el array.

	pid_t pid = fork();

	if (pid == 0) {  // HIJO

		res = execv(argumentos[0], argumentos);
		if (res > 0) {
			perror("EXECV failed\n");
			exit(EXIT_FAILURE);
		}

	} else if (pid > 0) {  // PADRE

		wait(NULL);

		*contador_argumentos =
		        0;  // Resetea el contador para el proximo grupo de argumentos

	} else {
		perror("FORK failed");
		exit(EXIT_FAILURE);
	}

	return;
}


// Pre: Recibe como parametro los argumentos enviados por linea de comandos.
// Post: -
void
aplicar_xargs(char *argv[])
{
	char *line = NULL;
	size_t len = 0;
	ssize_t nread = 0;

	char *argumentos[NARGS + 2];  // +2 por el nombre del programa y el NULL que finaliza
	int contador_argumentos = 0;

	argumentos[0] = argv[1];  // El nombre del programa que queremos correr

	while ((nread = getline(&line, &len, stdin)) > 0) {
		line[nread - 1] = '\0';

		argumentos[contador_argumentos + 1] =
		        line;  // Añade el argumento al vector
		contador_argumentos++;

		if (contador_argumentos == NARGS) {
			imprimir_argumentos(argumentos, &contador_argumentos);
		}

		line = NULL;
	}

	if (contador_argumentos > 0) {
		imprimir_argumentos(argumentos, &contador_argumentos);
	}

	free(line);

	return;
}


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("No se recibieron los argumentos correctamente\n");
		return ERROR;
	}

	aplicar_xargs(argv);

	return 0;
}