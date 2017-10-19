#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#include "svec.h"
#include "tokenizer.h"

int execute_tokens(char* cmd, svec* tokens, int redirect, int target, bool fork);

// Executes the given command and arguments, redirecting the output into
// the specified file.
int
redirect_output(char* cmd, svec* tokens)
{
    char* target = svec_get(tokens, svec_find(tokens, ">") + 1);
    int target_pid = open(target, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    int status = execute_tokens(cmd, svec_up_to(tokens, ">"), 1, target_pid, true);

    return status;
}

// Executes the given command and arguments,
// taking input from the specified file.
int
redirect_input(char* cmd, svec* tokens)
{
    char* target = svec_get(tokens, svec_find(tokens, "<") + 1);
    int target_pid = open(target, O_RDONLY);
    int status = execute_tokens(cmd, svec_up_to(tokens, "<"), 0, target_pid, true);

    return status;
}

// Executes the command before the '|', and redirects its output
// to the command after the '|'.

// Original Author: Nat Tuck
// (Significantly) Modified By: Me
int
execute_pipe(char* cmd, svec* tokens)
{
    int status;
    
    // Get left and right commands
    svec* left = svec_up_to(tokens, "|");
    svec* right = svec_up_from(tokens, "|");

    // Get the commands
    char* r_cmd = svec_get(tokens, svec_find(tokens, "|") + 1);
    char* l_cmd = cmd;

    int cpid;
    if ((cpid = fork())) {
        waitpid(cpid, &status, 0);
    }
    else {
        // Set up a pipe
        int pipe_fd[2];
        pipe(pipe_fd);

        int p_read = pipe_fd[0];
        int p_write = pipe_fd[1];
        
        int ccpid;
        if((ccpid = fork())) {
            close(p_write);
            
            // Read input from p_read
            close(0);
            dup(p_read);
            close(p_read);

            status = execute_tokens(r_cmd, right, -1, -1, false);

            exit(0);
        }
        else {
            close(p_read);

            // Execute the left side, writing to p_write
            status = execute_tokens(l_cmd, left, 1, p_write, false);
        }
    }

    return status;
}

// Executes the command after '&&' if and only if the command
// before '&&' executes successfully.
int
execute_and(char* cmd, svec* tokens)
{
    int status;

    svec* left = svec_up_to(tokens, "&&");
    status = execute_tokens(svec_get(left, 0), left, -1, -1, true);

    if (status == 0) {
        svec* right = svec_up_from(tokens, "&&");
        char* r_cmd = svec_get(right, 0);

        status = execute_tokens(r_cmd, right, -1, -1, true);
    }

    return status;
}

// Executes the command after '||' if and only if the command
// before '||' fails to execute successfully.
int
execute_or(char* cmd, svec* tokens)
{
    int status;

    svec* left = svec_up_to(tokens, "||");
    status = execute_tokens(svec_get(left, 0), left, -1, -1, true);

    if (status != 0) {
        svec* right = svec_up_from(tokens, "||");
        char* r_cmd = svec_get(right, 0);

        status = execute_tokens(r_cmd, right, -1, -1, true);
    }

    return status;
}

// Executes the command before the ';', then executes the
// command after the ';'.
int
execute_semicolon(char* cmd, svec* tokens)
{
    svec* left = svec_up_to(tokens, ";");
    execute_tokens(cmd, left, -1, -1, true);

    svec* right = svec_up_from(tokens, ";");
    char* r_cmd = svec_get(right, 0);
    
    return execute_tokens(r_cmd, right, -1, -1, true);
}

// Executes the given command in a background process.
void
execute_in_background(char* cmd, svec* tokens)
{
    int cpid;
    if (!(cpid = fork())) {
        execute_tokens(cmd, svec_up_to(tokens, "&"), -1, -1, false);
    }

}

// Executes the given command without forking, redirecting the input or
// output as specified.
int
execute_command_without_fork(char* cmd, svec* tokens, int redirect, int target)
{
        if (redirect != -1) {
            close(redirect);
            dup2(target, redirect);
            close(target);
        }

        char* args[tokens->size + 1];
        for (int ii = 0; ii < tokens->size; ++ii) {
            args[ii] = svec_get(tokens, ii);
        }
        args[tokens->size] = 0;

        int status = execvp(cmd, args);
        
        return status;
}

// Executes the given command by forking, redirecting the input or
// output as specified.
int
execute_command(char* cmd, svec* tokens, int redirect, int target)
{
    int status;

    int cpid;
    if ((cpid = fork())) {
        waitpid(cpid, &status, 0);
    }
    else {
        status = execute_command_without_fork(cmd, tokens, redirect, target);
    }

    return status;
}

// Executes the given command based on its operators, or lack thereof, redirecting
// the input or output as specified, and by forking if specified.
int
execute_tokens(char* cmd, svec* tokens, int redirect, int target, bool fork)
{
    int status = -1;
    
    for (int ii = 0; ii < strlen(cmd); ++ii) {
        if (cmd[ii] == ' ' || cmd[ii] == '\n') {
            cmd[ii] = 0;
            break;
        }
    }
    
    // Command: exit
    if (strcmp(cmd, "exit") == 0) {
        free_svec(tokens);
        exit(0);
    }
    // Command: cd
    else if (strcmp(cmd, "cd") == 0) {
        status = chdir(svec_get(tokens, 1));
    
        return status;
    }
    // ; Operator
    else if (svec_contains(tokens, ";")) {
        status = execute_semicolon(cmd, tokens);
    }
    // && Operator
    else if (svec_contains(tokens, "&&")) {
        status = execute_and(cmd, tokens);
    }
    // || operator
    else if (svec_contains(tokens, "||")) {
        status = execute_or(cmd, tokens);
    }
    // | Operator
    else if (svec_contains(tokens, "|")) {
        status = execute_pipe(cmd, tokens);
    }
    // & operator
    else if (svec_contains(tokens, "&")) {
        execute_in_background(cmd, tokens);
    }
    // Redirect Output
    else if (svec_contains(tokens, ">")) {
        status = redirect_output(cmd, tokens);
    }
    // Redirect Input
    else if (svec_contains(tokens, "<")) {
        status = redirect_input(cmd, tokens);
    }
    // Basic Command With Fork
    else if (fork) {
        status = execute_command(cmd, tokens, redirect, target);
    }
    // Basic Command Without Fork
    else {
        status = execute_command_without_fork(cmd, tokens, redirect, target);
    }

    free_svec(tokens);

    return status;
}

// Executes the given command.
void
execute(char* cmd)
{
    svec* tokens = tokenize(cmd);

    execute_tokens(cmd, tokens, -1, -1, true);
}

// Runs a command shell, from either stdin or a given script.
// Original Author: Nat Tuck
// Modified By: Me
int
main(int argc, char* argv[])
{
    char cmd[256];

    if (argc == 1) {
        for(;;) {
            char* green_color = "\x1b[32m";
            char* end_color = "\x1b[0m";

            printf("%snush$ %s", green_color, end_color);
            fflush(stdout);
            fgets(cmd, 256, stdin);

            if (feof(stdin)) {
                break;
            }

            execute(cmd);
        }
    }
    else {
        FILE* script = fopen(argv[1], "r");

        while(fgets(cmd, sizeof(cmd), script)) {
            execute(cmd);
        }
    }

    return 0;
}
