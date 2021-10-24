// Posted to https://stackoverflow.com/questions/69699756/parse-command-line-arguments-string-into-array-for-posix-spawn-execve/ ...
// ...but then the OP changed their question to requiring quoting and escaping,
// so I deleted my answer there

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h> // For _POSIX_ARG_MAX
#include <unistd.h> // For execve()
#include <errno.h>

#define NULL_CMD -1
#define TOO_MANY_ARGS -2

int parse_cmd(const char* cmd, char* argv[], size_t argv_size);

int main()
{
    const size_t max_args = _POSIX_ARG_MAX + 1; // +1 for final NULL
    char* my_argv[max_args];
    int ret;

    ret = parse_cmd("prog arg1 arg2 arg3", my_argv, max_args);
    assert(ret == 4);
    assert(strcmp(my_argv[0], "prog") == 0);
    assert(strcmp(my_argv[1], "arg1") == 0);
    assert(strcmp(my_argv[2], "arg2") == 0);
    assert(strcmp(my_argv[3], "arg3") == 0);
    assert(my_argv[4] == NULL);

    ret = parse_cmd("", my_argv, max_args);
    assert(ret == 0);
    assert(my_argv[0] == NULL);

    ret = parse_cmd(NULL, my_argv, max_args);
    assert(ret == NULL_CMD);

    ret = parse_cmd("prog", my_argv, max_args);
    assert(ret == 1);
    assert(strcmp(my_argv[0], "prog") == 0);
    assert(my_argv[1] == NULL);

    ret = parse_cmd("prog arg1", my_argv, max_args);
    assert(ret == 2);
    assert(strcmp(my_argv[0], "prog") == 0);
    assert(strcmp(my_argv[1], "arg1") == 0);
    assert(my_argv[2] == NULL);

    printf("All assertions passed.\n");

    printf("Now being ls:\n");

    ret = parse_cmd("/usr/bin/ls -l", my_argv, max_args);
    assert(ret > 0);
    char* envp[] = {NULL};
    execve(my_argv[0], my_argv, envp);

    // If execution reaches here, then execve failed
    fprintf(stderr, "execve failed: %s\n", strerror(errno));
    return 1;
}

int parse_cmd(const char* cmd, char* argv[], size_t max_args)
{
    if (cmd == NULL) {
        return NULL_CMD;
    }

    // Copy cmd into cmd_copy
    // This is done because strtok_r() may modify its input string
    const size_t cmd_copy_size = 4096;
    char cmd_copy[cmd_copy_size];
    strncpy(cmd_copy, cmd, cmd_copy_size);

    size_t argc = 0;
    char* saveptr;
    char* token;

    token = strtok_r(cmd_copy, " ", &saveptr);

    while (token != NULL) {
        if (argc == max_args) {
            return TOO_MANY_ARGS;
        }
        argv[argc++] = token;
        token = strtok_r(NULL, " ", &saveptr);
    }

    // Final entry must be NULL
    argv[argc] = NULL;

    return argc;
}
