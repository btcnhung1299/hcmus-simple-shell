#include <unistd.h>     // fork()
#include <sys/wait.h>   // waitpid()
#include <stdio.h>      // printf(), fgets()
#include <string.h>     // strtok(), strcmp(), strdup()
#include <stdlib.h>     // free()
#include <fcntl.h>      // open(), creat(), close()

using namespace std;
const unsigned MAX_LINE_LENGTH = 100;
const unsigned BUF_SIZE = 50;
const unsigned REDIR_SIZE = 2;
const unsigned MAX_HISTORY = 30;
const unsigned MAX_COMMAND_NAME = 30;

void parse_command(char input[], char* argv[], int* wait) {
	for (unsigned idx = 0; idx < BUF_SIZE; idx++) {
		argv[idx] = NULL;
	}

   // Check for trailing & and remove if exists
   if (input[strlen(input) - 1] == '&') {
      *wait = 1;
      input[strlen(input) - 1] = '\0';
   }
   else {
      *wait = 0;
   }

   // Perform tokenization on input string
   const char *delim = " ";
   unsigned idx = 0;

   char *token = strtok(input, delim);
   while (token != NULL) {
      argv[idx++] = token;
      token = strtok(NULL, delim);
   }

   argv[idx] = NULL;

}

void parse_redir(char* argv[], char* redir_argv[]) {
   unsigned idx = 0;
	redir_argv[0] = NULL;
	redir_argv[1] = NULL;

   while (argv[idx] != NULL) {

      // Check if command contains character <, >
      if (strcmp(argv[idx], "<") == 0 || strcmp(argv[idx], ">") == 0) {

         // Check for succeeded file name
         if (argv[idx + 1] != NULL) {

            // Move redirect type and file name to redirect arguments vector
            redir_argv[0] = strdup(argv[idx]);
      	   redir_argv[1] = strdup(argv[idx + 1]);
            argv[idx] = NULL;
            argv[idx + 1] = NULL;
         }
      }

      idx++;
   }
}


void child(char* argv[], char* redir_argv[]) {
   int fd_out, fd_in;

   if (redir_argv[0] != NULL) {
         
      // Redirect output
      if (strcmp(redir_argv[0], ">") == 0) {

         // Get file description
         fd_out = creat(redir_argv[1], S_IRWXU);
         if (fd_out == -1) {
            perror("Redirect output failed");
         }

         // Replace stdout with output file
         dup2(fd_out, STDOUT_FILENO);

         // Check for error on close
         if (close(fd_out) == -1) {
            perror("Closing output failed");
         }
      }

      // Redirect input
      else if (strcmp(redir_argv[0], "<") == 0) {

         // Get file description
         fd_in = open(redir_argv[1], O_RDONLY);
         if (fd_in == -1) {
            perror("Redirect input failed");
         }

         // Replace stdin with input from file
         dup2(fd_in, STDIN_FILENO);

         // Check for error on close
         if (close(fd_in) == -1) {
            perror("Closing input failed");
         }
      }
   }

   // Execute user command in child process
   if (execvp(argv[0], argv) == -1) {
      perror("Fail to execute command");
   }
}


void parent(pid_t child_pid, int wait) {
   int status;
   printf("Parent <%d> spawned a child <%d>.\n", getpid(), child_pid);
   
   switch (wait) {

      // Parent and child are running concurrently
      case 0: {
         waitpid(child_pid, &status, 0);
         break;
      }

      // Parent waits for child process with PID to be terminated
      default: {
         waitpid(child_pid, &status, WUNTRACED);

         if (WIFEXITED(status)) {   
            printf("Child <%d> exited with status = %d.\n", child_pid, status);
         }
         break;
      }
   }
}

void add_history_feature(char *history[], int &history_count, char* user_input) {
   // If history_count exceeds MAX_HISTORY, overwrite the last command

   if (history_count < MAX_HISTORY) {
      strcpy(history[history_count++], user_input);
   } 
   else {
      free(history[0]);
      for (int i = 1; i < MAX_HISTORY; i++) {
         strcpy(history[i - 1], history[i]);
		}
      strcpy(history[MAX_HISTORY - 1], user_input);
   }
}


int main() {
   bool running = true;
   pid_t pid;
   int status = 0, history_count = 0, wait;
   char user_input[MAX_LINE_LENGTH];
   char *argv[BUF_SIZE], *redir_argv[REDIR_SIZE], *history[MAX_HISTORY];

   for (int i = 0; i < MAX_HISTORY; i++)
		history[i] = (char*)malloc(MAX_COMMAND_NAME);

   while (running) {
      printf("osh>");
      fflush(stdout);
  
      // Read and parse user input
      while (fgets(user_input, MAX_LINE_LENGTH, stdin) == NULL) {
         perror("Cannot read user input!");
         fflush(stdin);
      }

    	// Remove trailing endline character
      user_input[strcspn(user_input, "\n")] = '\0';

      // Check if user entered "exit"
      if (strcmp(user_input, "exit") == 0) {
         running = false;
         continue;
      }

      // Check if user entered "!!"
      if (strcmp(user_input, "!!") == 0){
         if (history_count == 0)
            {
               fprintf(stderr, "No commands in history\n");
               continue;
            }
         strcpy(user_input, history[history_count - 1]);
         printf("osh>%s\n", user_input);
      }

      add_history_feature(history, history_count, user_input);
      parse_command(user_input, argv, &wait);
		parse_redir(argv, redir_argv);
      
      // Fork child process
      pid_t pid = fork();

      // Fork return twice on success: 0 - child process, > 0 - parent process
      switch (pid) {
         case -1:
            perror("fork() failed!");
            exit(EXIT_FAILURE);
      
         case 0:     // In child process
            child(argv, redir_argv);
            exit(EXIT_SUCCESS);
      
         default:    // In parent process
            parent(pid, wait);
      }
   }
   
   return 0;
}
