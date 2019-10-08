#include <unistd.h>     // fork()
#include <sys/wait.h>   // waitpid()
#include <stdio.h>      // printf(), fgets()
#include <string.h>     // strtok()
#include <stdlib.h>     // free()
using namespace std;
const int MAX_LINE_LENGTH = 200;
const int BUF_SIZE = 100;

char **parse(char *input, int *wait) {
   // Allocate new array for arguments
   char **argv = (char **)malloc(BUF_SIZE * sizeof(char *));

   // Remove trailing endline character
   input[strcspn(input, "\n")] = '\0';

   // Check if the last character is &
   *wait = (input[strlen(input) - 1] == '&' ? 1 : 0);

   // Perform tokenization on input string
   const char *delim = " ";
   unsigned idx = 0;

   char *token = strtok(input, delim);
   while (token != NULL) {
      argv[idx++] = token;
      token = strtok(NULL, delim);
   }
   argv[idx] = NULL;

   return argv;
}

void child(char** argv) {
   if (execvp(argv[0], argv) == -1) {
      perror("Cannot execute command.");
   }
}

void parent() {
   int status;
   wait(&status);

   if (WIFEXITED(status)) {
      printf("Child process exited with status %d.", status);
   }
}

int main() {
   bool running = true;
   pid_t pid, w_pid;
   int status = 0, wait;
   char **argv;
   char *user_input;

   while (running) {
      printf("osh>");
      fflush(stdout);
      
      // Read and parse user input
      while (fgets(user_input, MAX_LINE_LENGTH, stdin) == NULL) {
         perror("Cannot read user input!");
         fflush(stdin);
      }

      argv = parse(user_input, &wait);   
      
      // Fork child process
      pid_t pid = fork();

      // Fork return twice on success
      // (0 - child process, > 0 - parent process)
      switch (pid) {
         case -1:
            perror("fork() failed!");
            exit(EXIT_FAILURE);
      
         case 0:     // In child process
            child(argv);
            exit(EXIT_FAILURE);
      
         default:    // In parent process
            parent();
            exit(EXIT_FAILURE);
      }

      free(user_input);
      free(argv);
   }

   return 0;
}
