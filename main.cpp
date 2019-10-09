#include <unistd.h>     // fork()
#include <sys/wait.h>   // waitpid()
#include <stdio.h>      // printf(), fgets()
#include <string.h>     // strtok(), strcmp(), strdup()
#include <stdlib.h>     // free()
#include <fcntl.h>      // open(), close()
using namespace std;
const int MAX_LINE_LENGTH = 200;
const int BUF_SIZE = 100;

char **parse_command(char *input, int *wait) {
   // Allocate new array for arguments
   char **argv = (char **)malloc(BUF_SIZE * sizeof(char *));

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

   return argv;
}

char **parse_redirect(char** argv) {
   unsigned idx = 0;
   char *redirect_type = NULL, *file_name = NULL;

   // Allocate new array for redirect arguments
   char **redirect_argv = (char **)malloc(2 * sizeof(char *));

   while (argv[idx] != NULL) {

      // Check if user_command contains character <, >
      if (strcmp(argv[idx], "<") == 0 || strcmp(argv[idx], ">") == 0) {

         // Check for succeeded file name
         if (argv[idx + 1] != NULL) {

            // Move redirect type and file name to new variables
            redirect_type = strdup(argv[idx]);
            file_name = strdup(argv[idx + 1]);
            argv[idx] = NULL;
            argv[idx + 1] = NULL;
         }
      }

      idx++;
   }

   // Point redirect arguments to redirect type and file name
   redirect_argv[0] = redirect_type;
   redirect_argv[1] = file_name;
   return redirect_argv;
}


void child(char** argv, char** redirect_argv) {
   int fd_out, fd_in;

   // Redirect output
/*   
   if (strcmp(redirect_argv[0], ">") == 0) {
      printf("Redirect output!\n");

      // Get file description, set flags to write and create if it doesn't exist
      fd_out = creat(redirect_argv[1], S_IRWXU);
      if (fd_out == -1) {
         perror("Redirect output failed");
      }

      // Replace stdout with output to file
      dup2(fd_out, STDOUT_FILENO);

      // Check for error on close
      if (close(fd_out) == -1) {
         perror("Closing output failed");
      }
   }

   // Redirect input
   else if (strcmp(redirect_argv[0], "<") == 0) {
      printf("Redirect input!\n");      

      // Get file description, set flag to read
      fd_in = open(redirect_argv[1], O_RDONLY);
      if (fd_in == -1) {
         perror("Redirect input failed");
      }

      // Replace stdin with input from file
      dup2(fd_in, STDIN_FILENO);

      // Check for error on close
      if (close(fd_in) == -1) {
         perror("Closing input failed");
      }
   }*/

   // Execute user command in child process
   if (execvp(argv[0], argv) == -1) {
      perror("Fail to execute command");
   }
}

void parent(pid_t child_pid, int wait) {
   int status;
   printf("Parent <%d> spawned a child <%d>.\n", getpid(), child_pid);
   printf("Wait = %d\n", wait);

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

int main() {
   bool running = true;
   pid_t pid;
   int status = 0, wait;
   char **argv, **redirect_argv;
   char *user_input;

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

      argv = parse_command(user_input, &wait);   
      redirect_argv = parse_redirect(argv);

      // Fork child process
      pid_t pid = fork();

      // Fork return twice on success: 0 - child process, > 0 - parent process
      switch (pid) {
         case -1:
            perror("fork() failed!");
            exit(EXIT_FAILURE);
      
         case 0:     // In child process
            child(argv, redirect_argv);
            exit(EXIT_SUCCESS);
      
         default:    // In parent process
            parent(pid, wait);
            free(argv);
      }
   }
   
   return 0;
}
