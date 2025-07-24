#include <stdio.h>      // For input/output functions like printf and fgets
#include <stdlib.h>     // For exit() and standard library functions
#include <string.h>     // For string manipulation like strtok()
#include <unistd.h>     // For getcwd(), fork(), execv(), etc.
#include <sys/wait.h>   // For waitpid()

#define MAX_LINE 1024 // Set a limit to length of command line
#define MAX_ARGS 64   // Max number of arguments supported

// Function to print the shell prompt (ex: /home/gavin$)
void print_prompt() {
    char cwd[1024];     // Buffer to store the working directory
    getcwd(cwd, sizeof(cwd)); // Grab the current working directory
    printf("%s$ ", cwd);  // Prints the shell prompt with the current directory
    fflush(stdout);   // Forces the prompt to appear immediately
}

// Function to read a line of input from the user
void read_command(char *buffer) {
    // fgets reads one line from stdin and stores it in buffer
    if (fgets(buffer, MAX_LINE, stdin) == NULL) { 
        // If input is NULL (like Ctrl+D), exit the shell
        printf("\n");
        exit(0);
    }

    // Remove the newline character at the end of the input (replace with null terminator)
    buffer[strcspn(buffer, "\n")] = '\0';
}

// Function to split the command line into arguments
// For example: input: "ls -al" -> args: ["ls", "-al", NULL]
void parse_command(char *input, char **args) {
    int i = 0;
    // strtok splits the input using spaces
    char *token = strtok(input, " ");

    // Keep splitting until there are no more tokens or until max args is hit
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;          // Save each token into the args array
        token = strtok(NULL, " ");  // Move to the next token
    }

    args[i] = NULL; // NULL-terminate the array so execv() knows where the args end
}

int main() {
    char command_line[MAX_LINE];    // Buffer to hold the raw command input
    char *args[MAX_ARGS];           // Array of strings (char pointers) for command and arguments

    // Infinite loop to keep the shell running until manually exited
    while (1) {
        print_prompt();     // Show the prompt and working directory
        read_command(command_line);     // Grab user input

        if (strlen(command_line) == 0) {
            continue;    // If nothing is entered, skip to next iteration
        }

        parse_command(command_line, args); // Break command line into command + arguments


        // Handle the built-in exit
        if (strcmp(args[0], "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        // Handle built-in cd
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "cd: expected argument\n");
            } else if (chdir(args[1]) != 0) {
                perror("cd");
            }
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // This is the child process
            execvp(args[0], args);
            perror("execvp failed");
            exit(1); // This is to exit if evecvp fails
        } else {
            // This is the parent process
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0; // Exit the shell (currently not reachable)
}