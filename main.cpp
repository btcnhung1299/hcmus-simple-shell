#include <unistd.h>     // fork()
#include <sys/wait.h>   // waitpid()
#include <stdio.h>      // printf(), fgets()
#include <string.h>     // strtok(), strcmp(), strcpy()
#include <stdlib.h>     // free()


using namespace std;
const int MAX_LINE_LENGTH = 200;
const int BUF_SIZE = 100;
const int MAX_COMMAND_NAME = 20;
const int MAX_HISTORY = 30;

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

int parse_redirect(char** argv, char* file_name) {
   unsigned idx = 0;
   int redirect_type = 0;

   while (argv[idx] != NULL) {

      // Check if user_command contains character <, >
      if (strcmp(argv[idx], "<") == 0 || strcmp(argv[idx], ">") == 0) {

         // Check for succeeded file name
         if (argv[idx + 1] != NULL) {

            // 1: read from file, -1: write to file
            redirect_type = (strcmp(argv[idx], "<") ? 1 : -1);
            argv[idx] = NULL;

            // Move file name to a new variable 
            file_name = strdup(argv[idx + 1]);
            argv[idx + 1] = NULL;
         }
      }

      idx++;
   }

   return redirect_type;
}


void child(char** argv) {

   // Execute user command in child process
   if (execvp(argv[0], argv) == -1) {
      perror("Cannot execute command.");
   }
}

void parent(pid_t child_pid, int *suspend) {
   int status;
   printf("Parent <%d> spawned a child <%d>.\n", getpid(), child_pid);

   switch (*suspend) {

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
void add_history_feature(char *history[], int &history_count, char* user_input)
{
   // If history_count exceeds MAX_HISTORY
   // overwrite the last command

   if (history_count < MAX_HISTORY)
   {
      history[history_count++] = strdup(user_input);
   } 
   else
   {
      free (history[0]);
      for (int i = 1; i < MAX_HISTORY; i++)
         history[i - 1] = history[i];
      
      history[MAX_HISTORY - 1] = strdup(user_input);
   }
}
int main() {
   bool running = true;
   pid_t pid, w_pid;
   int status = 0, wait, redirect_type;
   char **argv;
   char *user_input;
   char *scr_file;
   char *history[MAX_HISTORY];
   int history_count = 0;

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
         user_input = strdup(history[MAX_HISTORY - 1]);
      }

      add_history_feature(history, history_count, user_input);
      argv = parse_command(user_input, &wait);   
      redirect_type = parse_redirect(argv, scr_file);

      

      // Fork child process
      pid_t pid = fork();

      // Fork return twice on success: 0 - child process, > 0 - parent process
      switch (pid) {
         case -1:
            perror("fork() failed!");
            exit(EXIT_FAILURE);
      
         case 0:     // In child process
            child(argv);
            exit(EXIT_SUCCESS);
      
         default:    // In parent process
            parent(pid, &wait);
            free(argv);
      }
   }
   
   return 0;
}
