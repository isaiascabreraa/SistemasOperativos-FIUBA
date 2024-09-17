#include "exec.h"
#include <fcntl.h>

extern int status;

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	char key[100];
	char value[100];


	for (int i = 0; i < eargc; i++) {
		int index = block_contains(eargv[i], '=');

		get_environ_value(eargv[i], value, index);
		get_environ_key(eargv[i], key);

		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	int mode = S_IRUSR |
	           S_IWUSR;  // Permisos de lectura y escritura del usuario
	//

	if (flags & O_CREAT) {
		fd = open(file, flags, mode);
		// El flag O_CREAT en en open permite que si el archivo no
		// existe, este sea creado. El tercer argumento 'mode' indica
		// con que permisos se crea el archivo.
	} else {
		fd = open(file, flags);
	}

	if (fd == -1) {
		perror("open");
		return -1;
	}

	return fd;
}


// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;


	switch (cmd->type) {
	case EXEC: {
		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		perror(e->argv[0]);
		_exit(EXIT_FAILURE);

		break;
	}
	case BACK: {
		b = (struct backcmd *) cmd;
		e = (struct execcmd *) b->c;

		execvp(e->argv[0], e->argv);

		break;
	}

	case REDIR: {
		r = (struct execcmd *) cmd;

		/*
		    Flags:
		    O_RDONLY sirve para que un archivo se abra solo para lectura.
		    O_WRONLY sieve para que un archivo se abra solo para escritura.
		    O_CREAT sirve para que si un archivo no existe, ese se cree.
		*/

		if (r->in_file[0]) {  // Si hay un archivo de entrada
			int fd_in = open_redir_fd(r->in_file, O_RDONLY);

			if (fd_in < 0) {
				perror("open in_file");
				_exit(EXIT_FAILURE);
			}
			dup2(fd_in, STDIN_FILENO);
			close(fd_in);
		}

		if (r->out_file[0]) {  // Si hay un archivo de salida
			int fd_out = open_redir_fd(r->out_file,
			                           O_WRONLY | O_CREAT | O_TRUNC);
			if (fd_out < 0) {
				perror("open out_file");
				_exit(EXIT_FAILURE);
			}
			dup2(fd_out, STDOUT_FILENO);
			close(fd_out);
		}

		if (r->err_file[0]) {  // Si hay un archivo de error
			if (strcmp(r->err_file, "&1") == 0) {
				int fd_err = open_redir_fd(r->err_file,
				                           O_CREAT | O_WRONLY);
				dup2(fd_err, STDERR_FILENO);
				dup2(STDOUT_FILENO, STDERR_FILENO);
			} else {
				int fd_err = open_redir_fd(r->err_file,
				                           O_CREAT | O_WRONLY);
				if (fd_err < 0) {
					perror("open err_file");
					_exit(EXIT_FAILURE);
				}
				dup2(fd_err, STDERR_FILENO);
				close(fd_err);
			}
		}

		execvp(r->argv[0], r->argv);

		free_command(cmd);

		perror("execvp");
		_exit(EXIT_FAILURE);
		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;

		struct sigaction act2;
		act2.sa_flags = SA_RESTART;
		act2.sa_handler = SIG_DFL;

		sigaction(SIGCHLD, &act2, NULL);

		int fds_pipe[2];
		int ret = pipe(fds_pipe);
		if (ret == -1) {
			perror("pipe");
		}

		pid_t pid = fork();

		if (pid == 0) {
			close(fds_pipe[0]);
			dup2(fds_pipe[1], STDOUT_FILENO);
			close(fds_pipe[1]);
			exec_cmd(p->leftcmd);
			_exit(EXIT_FAILURE);
		}

		pid_t pid2 = fork();

		if (pid2 == 0) {
			close(fds_pipe[1]);
			dup2(fds_pipe[0], STDIN_FILENO);
			close(fds_pipe[0]);
			exec_cmd(p->rightcmd);
			_exit(EXIT_FAILURE);
		}

		close(fds_pipe[READ]);
		close(fds_pipe[WRITE]);

		waitpid(pid, &status, 0);

		waitpid(pid2, &status, 0);
		// pid_t right_pid = fork();

		// printf("Pipes are not yet implemented\n");

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);

		_exit(EXIT_FAILURE);

		break;
	}
	}
}