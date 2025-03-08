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


int last_return_code = 0;  

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = (char *)malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }
    clear_cmd_buff(cmd_buff);
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}
int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    return OK;
}
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    clear_cmd_buff(cmd_buff);
    while (isspace(*cmd_line)) cmd_line++;
    
    if (*cmd_line == '\0') {
        return WARN_NO_CMDS;
    }
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    
    char *token = cmd_buff->_cmd_buffer;
    bool in_quotes = false;
    
    while (*token && cmd_buff->argc < CMD_ARGV_MAX - 1) {
     
        while (isspace(*token) && !in_quotes) token++;
        
        if (*token == '\0') break;
        
        cmd_buff->argv[cmd_buff->argc++] = token;
        
        while (*token) {
            if (*token == '"') {
                in_quotes = !in_quotes;
                memmove(token, token + 1, strlen(token));
                continue;
            }
            if (isspace(*token) && !in_quotes) {
                *token = '\0';
                token++;
                break;
            }
            token++;
        }
    }
    
    cmd_buff->argv[cmd_buff->argc] = NULL;
    return OK;
}
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        printf("cmd loop returned 0\n");
        exit(0);
    }
    if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc > 1) {
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd");
                last_return_code = 1;
            } else {
                last_return_code = 0;
            }
        } else {
            last_return_code = 0; 
        }
        return BI_EXECUTED;
    }
    if (strcmp(cmd->argv[0], "dragon") == 0) {
        print_dragon();
        last_return_code = 0;
        return BI_EXECUTED;
    }
    if (strcmp(cmd->argv[0], "rc") == 0) {
        printf("%d\n", last_return_code);
        last_return_code = 0;
        return BI_EXECUTED;
    }
    return BI_NOT_BI;
}
int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    cmd_buff_t cmd;
    int rc;

    if (alloc_cmd_buff(&cmd) != OK) {
        return ERR_MEMORY;
    }

    while (1) {
        printf("%s", SH_PROMPT);
        
        if (fgets(cmd_line, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_line[strcspn(cmd_line, "\n")] = '\0';

        rc = build_cmd_buff(cmd_line, &cmd);
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }
        if (exec_built_in_cmd(&cmd) == BI_EXECUTED) {
            continue;
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            last_return_code = 1;
            continue;
        }
        
        if (pid == 0) {
        
            execvp(cmd.argv[0], cmd.argv);
            fprintf(stderr, "Command not found\n"); 
            exit(127);
                
        } else {
           
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                last_return_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                last_return_code = 128 + WTERMSIG(status);
            } else {
                last_return_code = 1;
            }
        }
    }
    free_cmd_buff(&cmd);
    return OK;
}
