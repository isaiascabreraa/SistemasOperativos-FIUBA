#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

char prompt[PRMTLEN] = { 0 };
const stack_t stack_alternativo;

void
sigchild_handler()
{
	pid_t pid;
	ssize_t ret;
	char buffer[BUFLEN] = { 0 };

	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		snprintf(buffer, sizeof buffer, "==> terminado: PID=%d\n", pid);
		ret = write(STDOUT_FILENO, buffer, sizeof buffer);

		if (ret == -1) {
			perror("write");
		}
	}
}

// runs a shell command
static void
run_shell()
{
	char *cmd;
	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}

	sigaltstack(&stack_alternativo, NULL);

	struct sigaction act;
	act.sa_flags = SA_RESTART;
	act.sa_handler = sigchild_handler;

	sigaction(SIGCHLD, &act, NULL);
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
