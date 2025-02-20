#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

extern void print_dragon();

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX + 1);
    if (!cmd_buff->_cmd_buffer) 
        return ERR_MEMORY;
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
    }
    cmd_buff->_cmd_buffer = NULL;
    cmd_buff->argc = 0;
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    char *input = cmd_line;
    int num_tokens = 0;
    int in_quotes = 0;
    while (isspace((unsigned char)*input)) {
        input++;
    }

    if (*input == '\0') {
        return WARN_NO_CMDS;
    }

    char *cursor = input;
    char *token_start = input;
    char *token_store[CMD_ARGV_MAX];

    while (*cursor) {
        if (*cursor == '"') {
            in_quotes = !in_quotes;
            if (!in_quotes) {
                token_store[num_tokens] = strndup(token_start, cursor - token_start);
                if (++num_tokens >= CMD_ARGV_MAX) {
                    goto too_many_tokens;
                }
                while (isspace((unsigned char)*++cursor)) {
                 
                }
                token_start = cursor;
                continue;
            }
            token_start = cursor + 1;
        }
        else if (!in_quotes && isspace((unsigned char)*cursor)) {
      
            if (cursor > token_start) {
                token_store[num_tokens] = strndup(token_start, cursor - token_start);
                if (++num_tokens >= CMD_ARGV_MAX) {
                    goto too_many_tokens;
                }
            }
        
            while (isspace((unsigned char)*cursor)) {
                cursor++;
            }
            token_start = cursor;
            continue;
        }
        cursor++;
    }
    if (cursor > token_start && !in_quotes) {
        token_store[num_tokens] = strndup(token_start, cursor - token_start);
        if (++num_tokens >= CMD_ARGV_MAX) {
            goto too_many_tokens;
        }
    }
    if (in_quotes) {
        for (int i = 0; i < num_tokens; i++) {
            free(token_store[i]);
        }
        return ERR_CMD_ARGS_BAD;
    }

    size_t buf_size = 0;
    for (int i = 0; i < num_tokens; i++) {
        buf_size += strlen(token_store[i]) + 1;
    }

    free_cmd_buff(cmd_buff);
    cmd_buff->_cmd_buffer = malloc(buf_size);
    if (!cmd_buff->_cmd_buffer) {
        for (int i = 0; i < num_tokens; i++) {
            free(token_store[i]);
        }
        return ERR_MEMORY;
    }

    char *dest = cmd_buff->_cmd_buffer;
    cmd_buff->argc = num_tokens;
    for (int i = 0; i < num_tokens; i++) {
        strcpy(dest, token_store[i]);
        cmd_buff->argv[i] = dest;
        dest += strlen(token_store[i]) + 1;
        free(token_store[i]);
    }
    cmd_buff->argv[num_tokens] = NULL;
    return OK;

too_many_tokens:
    for (int i = 0; i < num_tokens; i++) {
        free(token_store[i]);
    }
    return ERR_TOO_MANY_COMMANDS;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (!cmd->argv[0]) {
        return BI_NOT_BI;
    }
    if (strcmp(cmd->argv[0], "exit") == 0) {
        return BI_CMD_EXIT;
    }
    else if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc > 1 && chdir(cmd->argv[1]) != 0) {
            perror("cd error");
        }
        return BI_CMD_CD;
    }
    else if (strcmp(cmd->argv[0], "dragon") == 0) {
        print_dragon();
        return BI_CMD_DRAGON;
    }
    return BI_NOT_BI;
}


int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork error");
        return ERR_EXEC_CMD;
    } 
    else if (pid == 0) {

        execvp(cmd->argv[0], cmd->argv);
        fprintf(stderr, "The command does not found\n");
        exit(EXIT_FAILURE);
    } 
    else {
    
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int exec_local_cmd_loop() {
    char input[SH_CMD_MAX];
    cmd_buff_t cmd;

    while (1) {
     
        printf("%s", SH_PROMPT);
        if (!fgets(input, SH_CMD_MAX, stdin)) {
   
            printf("\n");
            break;
        }
        input[strcspn(input, "\n")] = '\0'; 
        int parse_status = build_cmd_buff(input, &cmd);
        if (parse_status != OK) {
        
            fprintf(stderr, "%s",
                    parse_status == WARN_NO_CMDS
                        ? CMD_WARN_NO_CMD
                        : CMD_ERR_PIPE_LIMIT);
            continue;
        }

        Built_In_Cmds bic = exec_built_in_cmd(&cmd);
        if (bic == BI_CMD_EXIT) {
           
            free_cmd_buff(&cmd);
            break;
        }
        else if (bic == BI_NOT_BI) {

            int rc = exec_cmd(&cmd);
            if (rc != OK) {
                fprintf(stderr, "The command exited with the status %d\n", rc);
            }
        }

        free_cmd_buff(&cmd);
    }

    return OK;
}
