#include <unistd.h>     // fork()
#include <stdio.h>      // printf(), fgets()
#include <string.h>     // strtok()
using namespace std;
const int BUF_SIZE = 80;

void parse(char *input, char **argv) {
   // Remove trailing endline character
   input[strcspn(input, "\n")] = '\0';

   const char *delim = " ";
   unsigned idx = 0;

   char *token = strtok(input, delim);
   while (token != nullptr) {
      argv[idx++] = token;
      token = strtok(nullptr, delim);
   }

   argv[idx] = nullptr;
}

int main() {
   bool end_process = false;
   char *argv[BUF_SIZE];
   char *user_input;
   pid_t pid;

   while (!end_process) {
      printf("osh>");
      fflush(stdout);

      if (fgets(user_input, BUF_SIZE, stdin) == NULL) {
         perror("Cannot read user input!");
      }
      parse(user_input, argv);
      
      // Fork child process
      pid_t pid = fork();
      if (pid < 0) {
         perror("fork() failed!");
         exit(1);
      }
      else if (pid == 0) {
         execvp(argv[0], argv);
      }
   }

   return 0;
}
