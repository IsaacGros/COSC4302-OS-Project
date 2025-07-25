/*
Authors: Gavin Guyote, Isaac Gros, Oscar Herrera
Course: Summer 2025 Operating Systems (COSC-4302-48F)_OL
Assignment: COSC4302 Operating Systems Group Project
Instructor: Dr. Bo Sun
Due Date: 10:00am, July 28, 2025 (Monday)

Description of the problem the program was written to solve:
This group project was assigned to us with the intention of creating an interactive shell program
that prompts the user for a command, parses the command, and executes it with a child process.
The problem statement is that when users are operating a computer they need to interact with the
system OS for processing I/O such as running programs or managing files. However a system's OS is
made with the intention of being general purpose and may not provide the features for specific 
user's needs. So a custom shell can be designed with a specialized interface for the user which
works by invoking the OS services while also keeping the functionality of the OS intact.

The algorithm used to solve the problem:
Firstly a minishell or subshell needed to be created to simulate the behavior of a shell within a
shell in a Linux environment. Next the minishell needs to be able to execute computations so to 
handle this requirement a new child process is made execute commands. Executing commands with a 
child process instead of the parent process is done to prevent the initial process from crashing 
the machine if any fatal errors occur during execution. Next the minishell needed to get a command 
line in order to return any typed command by the user to the minishell. The minishell is able to
do this by performing a blocking read operation so that the is blocked until the usser types a
command to the minishell prompt. Once the minishell gets the command line it needs to parse the
command by reading the PATH variable entered and then building an array, dirs[] of the directories
in PATH. Next the minishell needs to be able to find the command file in order to execute the
command. The minishell searches the PATH environment variable either with a relative path or
absolute path in a list of absolute pathnames where the minishell should search for the command.
The minishell checks for these pathnames in all the included directories such as .:/bin:/usr/bin.
Finally once the command file has been found and retrieved the command is executed using execv,
which performs the operation of the entered command like ls, pwd, or mkdir in minishell the same
as it would in the initial shelll.

Program's operational requirements:
Programming Langauge: C
Runtime Environment: Linux system
Compiler: gcc
Input information: To use the minishell just compile the source code main.c as follows
"gcc -o mysh main.c" and then enter "./mysh" to run the minishell.
Incomplete required features: After reviewing the assigned materials and requirements.
We believe there are no missing features or existing bugs that we are currently aware of.
*/


#include <stdio.h>      // For input/output functions like printf and fgets
#include <stdlib.h>     // For exit() and standard library functions
#include <string.h>     // For string manipulation like strtok()
#include <unistd.h>     // For getcwd(), fork(), execv(), etc.
#include <sys/wait.h>   // For waitpid()

#define MAX_LINE 1024 // Set a limit to length of command line
#define MAX_ARGS 64   // Max number of arguments supported
#define MAX_PATHS 64  // Max number of directories in PATH
#define MAX_PATH_LEN 1024 // Max length for a full executable path


/*Class Name: print_prompt
Authors: Gavin
External Packages: <stdio.h>, <unistd.h>
Class description: Function to print the shell prompt (ex: /home/gavin$)
*/
void print_prompt() {
    char cwd[1024]; // Array to store entered name of current working directory
    getcwd(cwd, sizeof(cwd)); // Grab the current working directory
    printf("%s$ ", cwd);  // Show current directory as prompt
    fflush(stdout);   // Forces the prompt to appear immediately
}

/*Class Name: read_command
Authors: Gavin
External Packages: <stdio.h>, <string.h>, <unistd.h>
Class description: Function to read a line of input from the user
*/
void read_command(char *buffer) {
    if (fgets(buffer, MAX_LINE, stdin) == NULL) { 
        // If input is NULL (like Ctrl+D), exit the shell
        printf("\n");
        exit(0);
    }

    // Remove the newline character at the end of the input (replace with null terminator)
    buffer[strcspn(buffer, "\n")] = '\0';
}

/*Class Name: parse_command
Authors: Gavin
External Packages: <stdio.h>, <string.h>
Class description:  Function to split the command line into arguments
*/
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

/*Class Name: parse_path
Authors: Gavin
External Packages: <stdio.h>, <string.h>
Class description:  Function to parse the entered PATH to be looked up
*/
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

/*Class Name: *lookup_path
Authors: Gavin
External Packages: <stdio.h>, <string.h>, <unistd.h>
Class description:  Find the executable by looking in each folder from PATH
*/
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

/*Class Name: main
Authors: Gavin
External Packages: <stdio.h>, <string.h>, <unistd.h>, <sys/wait.h>
Class description:  Main function where the code is executed and processed by invoking helper functions
*/
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