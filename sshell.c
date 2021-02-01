#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>

#define CMDLINE_MAX 512
#define ARGCOUNT_MAX 16
#define ARGSIZE_MAX 32
#define CMDCOUNT_MAX 4

struct command_obj {
	char *args[ARGCOUNT_MAX + 1];
	char *output_file;
	int arg_count;
};

char *ignore_spaces(char *cmd) {
	while(isspace(*cmd))
		cmd++;
	return cmd;
}

bool parse_command(char *cmd_ptr, struct command_obj *commandobj_ptr, int *ampersand_ptr)
{
	char *out_redir;
	if (*cmd_ptr != '\0') {
		if ((*cmd_ptr) == '&') {
			*ampersand_ptr = 1;
			fprintf(stderr, "ampersand vale: %d\n", *ampersand_ptr);
			*(cmd_ptr) = ' ';
		} else if (strchr(cmd_ptr, '&')) {
			*ampersand_ptr = 2;
			*strchr(cmd_ptr, '&') = ' ';
		}
	} 
	cmd_ptr = ignore_spaces(cmd_ptr);
	char *track;
	while ((track = strchr(cmd_ptr, ' '))){
		commandobj_ptr->args[commandobj_ptr->arg_count] = cmd_ptr;
		*track = '\0';	
		cmd_ptr = ignore_spaces(track + 1);
		
		//find if there is output redirection
		if ((out_redir = strchr(cmd_ptr,'>'))) {
			*out_redir = '\0';
			out_redir = ignore_spaces(out_redir + 1);
			if (!(*out_redir))
				return true;
			if(strchr(out_redir,' ')) {
				*(strchr(out_redir,' ')) = '\0';
			}
			commandobj_ptr->output_file = out_redir;
		}
		(commandobj_ptr->arg_count)++;
	} 

	if (!track && *cmd_ptr != '\0') {
		commandobj_ptr->args[commandobj_ptr->arg_count] = cmd_ptr;
		if ((out_redir = strchr(cmd_ptr,'>'))) {
			*out_redir = '\0';
			out_redir = ignore_spaces(out_redir + 1);
			if (!(*out_redir))
				return true;
			if(strchr(out_redir,' ')) {
				*(strchr(out_redir,' ')) = '\0';
			}
			commandobj_ptr->output_file = out_redir;
		}			
		(commandobj_ptr->arg_count)++;
	}
	return false;
}

int main(void)
{
	char cmd[CMDLINE_MAX];

	while (1) {
		char *nl;
		char *cmd_ptr;
		int ampersand = 0;
		int *ampersand_ptr = &ampersand;
		char *pipe_present = NULL;
		pid_t pid;
		int status;
		int out_redir_fd;
		int piper[2];
		int retvals[CMDCOUNT_MAX];
		int input_pipe = STDIN_FILENO;
		int num_pipes = 0;
		bool carrot_pipe = false;
		bool too_many_args = false;
		bool redirection_error = false;
		bool pipe_prior = false;
		bool *pipe_prior_ptr = &pipe_prior;
		struct command_obj command;
		for (int k = 0; k < ARGCOUNT_MAX + 1; k++) 
			command.args[k] = NULL;
		
		command.arg_count = 0;
		command.output_file = NULL;
		struct command_obj *commandobj_ptr = &command;
		char original_full[CMDLINE_MAX];
		//fflush(stdin);
		
		// Print prompt
		printf("sshell$ ");
		fflush(stdout);

		// Get command line
		fgets(cmd, CMDLINE_MAX, stdin);

		// Print command line if stdin is not provided by terminal
		if (!isatty(STDIN_FILENO)) {
			printf("%s", cmd);
			fflush(stdout);
		}
		
		cmd_ptr = cmd;
		
		// Remove trailing newline from command line
		nl = strchr(cmd, '\n');
		if (nl)
			*nl = '\0';
		
		strcpy(original_full, cmd);
		
		for (int i = 0; i < (int)strlen(original_full); i++) {
			if (original_full[i] == '|')
				num_pipes++;
		}

		if (*cmd == '\0') {
			continue;
		} else if (!(*ignore_spaces(cmd))) {
			continue;
		}
		
		for (int pipe_iter = 0; pipe_iter < num_pipes; pipe_iter++) {
			struct command_obj command2;
			for (int k = 0; k < ARGCOUNT_MAX + 1; k++) 
				command2.args[k] = NULL;
			command2.arg_count = 0;
			command2.output_file = NULL;
			commandobj_ptr = &command2;

			pipe_present = strchr(cmd_ptr, '|');
			if (pipe_present) {
				*pipe_present = '\0';
			}
			redirection_error = parse_command(cmd_ptr, commandobj_ptr, ampersand_ptr);

			if (redirection_error)
				break;
			if (commandobj_ptr->output_file) {
				carrot_pipe = true;
				break;
			}
			// too many arguments check
			if (command2.arg_count > ARGCOUNT_MAX) {
				too_many_args = true;
				break;
			}
			pipe_prior = true;
			pipe(piper);
			pid = fork();
			if (pid == 0) {
				if (pipe_iter == 0) {
					dup2(piper[1], STDOUT_FILENO);
					if (ampersand == 1)
						dup2(piper[1], STDERR_FILENO);
				} else if (*pipe_prior_ptr && pipe_present) {
					dup2(piper[1], STDOUT_FILENO);
					dup2(input_pipe, STDIN_FILENO);
					if (ampersand == 1)
						dup2(piper[1], STDERR_FILENO);
				} else if (*pipe_prior_ptr && !pipe_present) {
					dup2(input_pipe, STDIN_FILENO);
					if (ampersand == 1)
						dup2(piper[1], STDERR_FILENO);
				}
				if (execvp(command2.args[0], command2.args) < 0) {
					fprintf(stderr, "Error: command not found\n");
					//fprintf(stderr, "+ completed '%s' [1]\n", original_full);
					exit(1);
				} 
			} else if (pid > 0) {  //parents
				wait(&status);
				retvals[pipe_iter] = WEXITSTATUS(status);
			} else {
				perror("fork didn't work");
				exit(1);
			}
			input_pipe = piper[0];

			if (pipe_present) {
				cmd_ptr = pipe_present + 1;
			}
			
			if (!pipe_present)
				close(piper[0]);
			close(piper[1]);
		}
		
		// Some Error Management
		if (too_many_args) {
			fprintf(stderr, "Error: too many process arguments\n");
			continue;
		}	
		
		if (redirection_error) {
			fprintf(stderr, "Error: no output file\n");
			continue;
		}
		if (carrot_pipe) {
			fprintf(stderr, "Error: mislocated output redirection\n");
			continue;
		}

		// Parse single/last commands into arguments
		commandobj_ptr = &command;
		pipe_present = strchr(cmd_ptr, '|');
		redirection_error = parse_command(cmd_ptr, commandobj_ptr, ampersand_ptr);
		if (redirection_error) {
			fprintf(stderr, "Error: no output file\n");
			continue;
		}
		if (command.arg_count > ARGCOUNT_MAX) {
			fprintf(stderr, "Error: too many process arguments\n");
			continue;
		}
		
		// Builtin command
		// Exit
		if (!strcmp(command.args[0], "exit")) {
			fprintf(stderr, "Bye...\n");
			fprintf(stderr, "+ completed 'exit' [0]\n");
			break;
		}
		
		// PWD Command
		char cwd[1000];
		bool checkpwd = false;
		if (!strcmp(command.args[0], "pwd")){
			checkpwd = true;
		}
		if (checkpwd){
			getcwd(cwd, sizeof(cwd));
			checkpwd = false;
			printf("%s\n", cwd);
			fprintf(stderr, "+ completed '%s' [0]\n", original_full);
			fflush(stdout);
			continue;
		}
		
		// CD Command
		if (!strcmp(command.args[0], "cd")){
			if (chdir(command.args[1]) == 0){
				fprintf(stderr, "+ completed '%s' [0]\n", original_full);
				continue;
			}
			else if(chdir(command.args[1]) !=0){
				fprintf(stderr, "Error: no such directory\n");
				fprintf(stderr, "+ completed '%s' [1]\n", original_full);
				continue;
			}
		}
		
		pipe(piper);
		pid = fork();
		if (pid == 0) {
			if (*pipe_prior_ptr && !pipe_present) {
				dup2(input_pipe, STDIN_FILENO);
			}
			if (commandobj_ptr->output_file) {
				if (open(commandobj_ptr->output_file, O_WRONLY) < 0) {
					open(commandobj_ptr->output_file, O_CREAT);
				}
				if (out_redir_fd < 0) {
					fprintf(stderr, "Error: cannot open output file\n");
					exit(1);
				}
				dup2(out_redir_fd = open(commandobj_ptr->output_file, O_WRONLY), STDOUT_FILENO);
				if (ampersand == 2)
					dup2(out_redir_fd, STDERR_FILENO);
			}
			if (execvp(command.args[0], command.args) < 0) {
				fprintf(stderr, "Error: command not found\n");
				//fprintf(stderr, "+ completed '%s' [1]\n", original_full);
				exit(1);
			}
		} else if (pid > 0) {  //parents
			wait(&status);
			if (ampersand > 0)
				close(out_redir_fd);
			fprintf(stderr, "+ completed '%s' ", original_full);
			for (int j = 0; j < num_pipes; j++) {
				fprintf(stderr, "[%d]", retvals[j]);
			}
			fprintf(stderr, "[%d]\n", WEXITSTATUS(status));
		} else {
			perror("fork didn't work");
			exit(1);
		}
		if (pipe_present) {
			cmd_ptr = pipe_present + 1;
		}

		
		close(piper[0]);
		close(piper[1]);
	}
	return EXIT_SUCCESS;
}
