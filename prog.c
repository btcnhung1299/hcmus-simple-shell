#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

const unsigned MAX_LINE_LENGTH = 200;
const unsigned BUF_SIZE = 100;
const unsigned REDIR_SIZE = 50;
const unsigned MAX_COMMAND_NAME = 20;
const unsigned MAX_HISTORY = 30;


void parse_command(char* input, char* argv[], int* wait);
void parse_redir(char* argv[], char* redir_argv[]);


int main(void) {
	int running = 1;
	char user_input[MAX_LINE_LENGTH];


	while (running) {
		printf("osh>");
		fflush(stdout);

		if (fgets(user_input, MAX_LINE_LENGTH, stdin) == NULL) {
			perror("Cannot read user input");
			exit(EXIT_FAILURE);
		}
		
	}

	return 0;
}
