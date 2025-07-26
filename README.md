# COSC4302-OS-Project
COSC4302 Operating Systems Group Project. 
Group Members: Gavin Guyote, Isaac Gros, Oscar Herrera.

This software was developed to provide a custom basic command line user interface
in the form of a linux shell that invokes the OS services. The main features that
were included was the ability to launchthe shell, prompt the user for input, enter
commands, parse the command, and execute the command with a child process using
fork() and execv(). This allows the user to interact with the system OS services
with operations like processing I/O, managing files and running software all with
freedom of a custom shell. The requirements to use the software are listed below.

Program's operational requirements:

Programming Language: C
Runtime Environment: Linux (WSL, Ubuntu)
Compiler: gcc (GNU Compiler Collection)
Input information:
As an example to use the minishell just compile the source code main.c as follows
"gcc -o mysh main.c" and then enter "./mysh" to run the minishell. The arg "mysh"
is an example name for the compiled file. The user is free to name the compiled
file whatever they want.