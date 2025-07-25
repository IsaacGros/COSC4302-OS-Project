#include <stdio.h>      // For input/output functions like printf and fgets
#include <stdlib.h>     // For exit() and standard library functions
#include <string.h>     // For string manipulation like strtok()
#include <unistd.h>     // For getcwd(), fork(), execv(), etc.
#include <sys/wait.h>   // For waitpid()

#define MAX_LINE 1024 // Set a limit to length of command line
#define MAX_ARGS 64   // Max number of arguments supported
#define MAX_PATHS 64  // Max number of directories in PATH
#define MAX_PATH_LEN 1024 // Max length for a full executable path

// Function to print the shell prompt (ex: /home/gavin$)
void print_prompt() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd)); // Grab the current working directory
    printf("%s$ ", cwd);  // Show current directory as prompt
    fflush(stdout);   // Forces the prompt to appear immediately
}

// Function to read a line of input from the user
void read_command(char *buffer) {
    if (fgets(buffer, MAX_LINE, stdin) == NULL) { 
        // If input is NULL (like Ctrl+D), exit the shell
        printf("\n");
        exit(0);
    }

    // Remove the newline character at the end of the input (replace with null terminator)
    buffer[strcspn(buffer, "\n")] = '\0';
}

// Function to split the command line into arguments
void parse_command(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " ");

    // Keep splitting until there are no more tokens or until max args is hit
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }

    args[i] = NULL; // NULL-terminate the array so execv() knows where to stop
}

int parse_path(char *dirs[]) {
    char *path_env = getenv("PATH");   // Grab the path variable
    int i = 0; 

    // strtok will destory the original string, duplicate safely
    char *path_copy = strdup(path_env);
    char *token = strtok(path_copy, ":");

    while (token != NULL && i < MAX_PATHS - 1) {
        dirs[i++] = strdup(token); // Copy each directory
        token = strtok(NULL, ":");
    }

    dirs[i] = NULL;
    free(path_copy); // Clean up temp copy
    return i;

}

// Find the executable by looking in each folder from PATH
char *lookup_path(char *command, char *dirs[]) {
    static char full_path[MAX_PATH_LEN];

    // Check if commmand already absolute
    if (command[0] == '/') {
        return command;
    }

    // Try each directory and command combination
    for (int i = 0; dirs[i] != NULL; i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dirs[i], command);
        if (access(full_path, X_OK) == 0) {
            return full_path; // Command found
        }
    }

    return NULL;    // Could not find command.
}


int main() {
    char command_line[MAX_LINE];    // Buffer to hold the raw command input
    char *args[MAX_ARGS];           // Array of strings (char pointers) for command and arguments
    char *path_dirs[MAX_PATHS];     // All folders from $PATH

    parse_path(path_dirs);

    // Infinite loop to keep the shell running until manually exited
    while (1) {
        print_prompt();     // Show the directory as prompt
        read_command(command_line);     // Wait for user input

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

        // Launc external command using fork and execv()
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // This is the child process
            char *full_path = lookup_path(args[0], path_dirs);

            if (full_path == NULL) {
                fprintf(stderr, "%s: command not found\n", args[0]);
                exit(1);
            }

            execv(full_path, args);
            perror("execv failed");  // This will print if and why execv fails
            exit(1);            // Exit child process
        } else {
            // This is the parent process
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0; // Exit the shell (by typing "exit")
}