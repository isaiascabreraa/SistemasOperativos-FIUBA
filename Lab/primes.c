
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define FD_READ 0   // file descriptor de lectura
#define FD_WRITE 1  // file descriptor de escritura
#define MODOS 2     // lectura y escritura

const int ERROR = -1;

// Pre: Recibe los punteros de un determinado pipe.
// Post: Devuelve (0) si todo esta bien, (-1) si algo falló.
int
procesos_sucesivos(int pipe_uno[MODOS], int n)
{
	int res;
	int valorLeido;
	int primerValor;

	int pipe_dos[MODOS];

	if (read(pipe_uno[FD_READ], &primerValor, sizeof(primerValor)) > 0) {
		printf("primo %i\n", primerValor);
		fflush(stdout);  // Limpia el buffer de stdout

		if (pipe(pipe_dos) < 0) {
			printf("Error al crear el pipe 2\n");
			return (ERROR);
		}

		pid_t pid2 = fork();

		if (pid2 > 0) {  // PADRE (EX HIJO)

			close(pipe_dos[FD_READ]);

			while ((read(pipe_uno[FD_READ],
			             &valorLeido,
			             sizeof(valorLeido))) > 0) {
				if (valorLeido > n) {
					return ERROR;
				}


				if ((valorLeido % primerValor) != 0) {
					res = write(pipe_dos[FD_WRITE],
					            &valorLeido,
					            sizeof(valorLeido));
					if (res < 0) {
						perror("Error de lectura\n");
						exit(EXIT_FAILURE);
					}
				}
			}

			close(pipe_dos[FD_WRITE]);
			close(pipe_uno[FD_READ]);

			wait(NULL);

		} else if (pid2 == 0) {  // HIJO (ACTUAL)

			close(pipe_dos[FD_WRITE]);
			close(pipe_uno[FD_READ]);

			if (procesos_sucesivos(pipe_dos, n) < 0) {
				perror("Error en lectura o escritura\n");
				return ERROR;
			}

			close(pipe_dos[FD_READ]);
		}
	}
	close(pipe_uno[FD_READ]);

	return 0;
}

// Pre: Recibe el valor del numero introducido por consola de comandos.
// Post: -
void
aplicar_primes(int n)
{
	int res;
	int pipe_uno[MODOS];

	if (pipe(pipe_uno) < 0) {
		perror("Error al crear el pipe 1\n");
		exit(ERROR);
	}

	pid_t pid = fork();

	if (pid > 0) {  // PADRE

		close(pipe_uno[FD_READ]);

		for (int i = 2; i <= n; i++) {
			res = write(pipe_uno[FD_WRITE], &i, sizeof(i));
			if (res < 0) {
				perror("Error de lectura\n");
				exit(ERROR);
			}
		}

		close(pipe_uno[FD_WRITE]);
		wait(NULL);


	} else if (pid == 0) {  // HIJO

		close(pipe_uno[FD_WRITE]);

		res = procesos_sucesivos(pipe_uno, n);
		if (res < 0) {
			perror("Error al ejectuar procesos sucesivos");
			return;
		}

		close(pipe_uno[FD_READ]);
	}
	return;
}


int
main(int argc, char *argv[])
{
	if (argc <= 0)
		return ERROR;

	if (argv == NULL)
		return ERROR;

	int n = atoi(argv[1]);

	aplicar_primes(n);

	return 0;
}