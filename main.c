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
1.A minishell or subshell needed to be created to simulate the behavior of a shell within a
shell in a Linux environment.
2.The minishell needs to be able to execute computations, so to handle this requirement a new
child process is made to execute commands. Executing commands with a child process instead of
the parent process is done to prevent the initial process from crashing the machine if any fatal
errors occur during execution.
3.The minishell needed to have a command line in order to prompt any typed command by the user to
the minishell. The minishell is able to do this by performing a blocking read operation so that
the shell is blocked until the user types a command to the minishell prompt.
4.Once the command is entered into the minishell it needs to parse the command into an array of
tokens that can be read as the command and its specified arguments by execv().
5.The minishell needs to be able to find where the command file actually exists on the disk in
order to execute the command. The minishell searches for the PATH environment variable like 
.:/bin:/usr/bin. for all the places where the command file could exist. This is done by creating
an array of directories for each directory with dirs[].
6.The minishell can now check each directory in the dirs[] array to find the tokenized command.
This is done by looping through each directory and invoking the function access() to search for
the command in the directory.
7.Once the location of the command is found the full absolute path can be used to run the command 
by entering the tokenized array of the command into the execv() function. This performs the
operation of the entered command like ls, pwd, or mkdir in minishell just the same as it would in
the initial shell.
8.The parent process, waits for the child process executing the command to terminate before
continuing the shell loop for the next command.

Program's operational requirements:
Programming Language: C
Runtime Environment: Linux system
IDE: Visual Studio Code
Repository: Github
Compiler: gcc (GNU Compiler Collection)

Input information: 
To use the minishell just compile the source code main.c as follows "gcc -o mysh main.c" and then
enter "./mysh" to run the minishell.

Incomplete required features: 
After reviewing the assigned materials and requirements. We believe there are no missing features
or existing bugs that we are currently aware of.
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


/*Function Name: print_prompt
Authors: Gavin, Oscar
External Packages: <stdio.h>, <unistd.h>
Function description: This function prints the current working directory as a
shell prompt (ex: /home/gavin$) emulating a Linux shell. This is to inform the
user the file path they are currently in as well as acting as the shell prompt
for the user to interface with for actions like entering a command.
*/

void print_prompt() {
    char cwd[1024]; // A character buffer that stores the full path of the current working directory.
    getcwd(cwd, sizeof(cwd)); // Grabs the current working directory and stores it in the cwd[] array.
    printf("%s$ ", cwd);  // Prints the working directory using %string with a $ appended to it for prompt format
    fflush(stdout);   // Forces the prompt to appear immediately
}

/*Function Name: read_command
Authors: Gavin, Oscar
External Packages: <stdio.h>, <string.h>, <unistd.h>
Function description: This function reads a line of input from the user and stores
it in a buffer. The function also includes an exit control that exits the shell.
The function also removes the newline character to handle the strings syntax conversion. 
*/
void read_command(char *buffer) { 
    // fgets reads the entered standard input in buffer up to the MAX_LINE characters
    if (fgets(buffer, MAX_LINE, stdin) == NULL) { 
        // If input is NULL (like Ctrl+D), exit the shell
        printf("\n");   // Move to a new line before exiting for formatting
        exit(0);    // Exits the shell
    }

    // strcspn finds the index of the '\n' char and replaces it with '\0', null
    // terminator. This removes the newline char left by fgets so that the string
    // does not read past the user's input.
    buffer[strcspn(buffer, "\n")] = '\0';
}

/*Function Name: parse_command
Authors: Gavin, Isaac
External Packages: <stdio.h>, <string.h>
Function description: This functions parses the raw input string entered by the
user and splits it into individual arguments, and are stored in the args[] array.
Each word is separated by a space which separates them into arguments. The array
is then NULL-terminated to make it compatible with execv() and similar functions
that require separated arguments in this format. 
*/
void parse_command(char *input, char **args) {
    int i = 0;  // Increment with i through the input buffer
    // Store the first word in token and split input on the first space
    char *token = strtok(input, " ");
    // Keep splitting until there are no more tokens or until max args is hit
    while (token != NULL && i < MAX_ARGS - 1) {
        // After separating store the args in the array
        args[i++] = token;  // Store the current token
        token = strtok(NULL, " ");  // By the next space get the next token
    }

    args[i] = NULL; // NULL-terminate the array so execv() knows where to stop
}

/*Function Name: parse_path
Authors: Gavin
External Packages: <stdio.h>, <string.h>, <stdlib.h>
Function description:  Function to parse the system's PATH environment variable,
which is a colon separated list of directories to be looked up and each directory
is stored in the dirs[] array. This allows the shell to manually search through
each of these directories that the entered command is expected to be stored in.
*/
int parse_path(char *dirs[]) {
    char *path_env = getenv("PATH");   // Grab the path variable
    int i = 0; 

    // strtok will destory the original string, duplicate safely
    // strup duplicates the current path variable
    char *path_copy = strdup(path_env);
    // strtok splits on the colon to separate the directories individually
    char *token = strtok(path_copy, ":");

    while (token != NULL && i < MAX_PATHS - 1) {
        dirs[i++] = strdup(token); // Copy each directory path into dirs[]
        token = strtok(NULL, ":");
    }

    dirs[i] = NULL;     // Null-terminate the array once parsing is done
    free(path_copy);    // Clean up temp copy string
    return i;           // Return number of directories parsed

}

/*Function Name: *lookup_path
Authors: Gavin, Isaac
External Packages: <stdio.h>, <string.h>, <unistd.h>
Function description: This function finds the executable by looking in each folder from PATH variable
previously stored dirs[]. This function is required when using execv() to execute commands because the
absolute path is required and the absolute path has to be manually looked up. If the command is found
in one of the directories it returns the full path to the executable. If the command is already an
absolute path, starts with '/', it is returned as it is. The function returns NULL if the command was
not found in the provided dirs[].
*/
char *lookup_path(char *command, char *dirs[]) {
    static char full_path[MAX_PATH_LEN]; // Buffer to build and store the full path of the command

    // Check if commmand already absolute and use it as it is
    if (command[0] == '/') {
        return command;
    }

    // Try each combination search by prepending each directory to the command name
    for (int i = 0; dirs[i] != NULL; i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dirs[i], command); // Build path like "/bin/ls"
        // Check if the combination path points to an executable file
        if (access(full_path, X_OK) == 0) {
            return full_path; // Command found
        }
    }

    return NULL;    // Could not find command if no matches were found 
}

/*Function Name: main
Authors: Gavin, Isaac, Oscar
External Packages: <stdio.h>, <string.h>, <unistd.h>, <sys/wait.h>
Function description:  Main function of the shell program. It displays a command prompt, reads user input,
and parses it into commands and arguments. The main function also handles some built in commands like 'cd'
and 'exit' directly. The external commands however are handled by shell, it forks a child process and uses
execv() to run the command by using the directory locations from the system PATH.

The main function uses the following logic:
1. Load and parse the PATH environment variable into a list of directories.
2. Enter an infinite loop to display the command prompt to handle user commands.
3. Explicit handling for 'exit' to stop the shell, and 'cd' to change directories.
4. All other commands are considered to be external and are executed with a child process.
5. The parent waits for the child to finish executing and terminate before prompting the user again.
*/
int main() {
    char command_line[MAX_LINE];    // Buffer to hold the raw command input
    char *args[MAX_ARGS];           // Array of strings (char pointers) for command and arguments
    char *path_dirs[MAX_PATHS];     // All folders from $PATH

    parse_path(path_dirs); // Load $PATH directories into path_dirs

    printf("Starting shell...\n"); // Start message to inform user subshell is running
    sleep(3); // Sleep to simulate boot load time
    printf("Start success\nPlease enter a command\n"); // Initial message to prompt the user

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

        // Launch external command using fork and execv()
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // Child process to try and execute the entered command
            char *full_path = lookup_path(args[0], path_dirs);

            if (full_path == NULL) {
                fprintf(stderr, "%s: command not found\n", args[0]);
                exit(1);
            }

            execv(full_path, args); // Replace the child with the command
            perror("execv failed"); // This will print if and why execv fails
            exit(1);                // Exit child process
        } else {
            // Parent process waits for the child process to finish
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0; // Successful return value when exiting the shell (by typing "exit")
}