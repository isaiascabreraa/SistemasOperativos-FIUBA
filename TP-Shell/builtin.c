#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0) {
		return 1;
	}

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (cmd[0] == 'c' && cmd[1] == 'd') {
		if ((strcmp(cmd, "cd") == 0) || (strcmp(cmd, "cd ") == 0)) {
			chdir(getenv("HOME"));
		} else {
			chdir(cmd + 3);
		}
		char working_directory[sizeof(prompt) - 2];
		getcwd(working_directory, sizeof(working_directory));
		snprintf(prompt, sizeof prompt, "(%s)", working_directory);
		return 1;
	}

	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	char working_directory[BUFLEN];
	if (strcmp(cmd, "pwd") == 0) {
		getcwd(working_directory, sizeof(prompt));
		printf("%s\n ", working_directory);
		return 1;
	}

	return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
