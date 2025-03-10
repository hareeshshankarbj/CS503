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

static int last_return_code = 0;

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer) {
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
    while (isspace((unsigned char)*cmd_line)) cmd_line++;
    if (*cmd_line == '\0') {
        return WARN_NO_CMDS;
    }
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    char *token = cmd_buff->_cmd_buffer;
    bool in_quotes = false;
    while (*token && cmd_buff->argc < CMD_ARGV_MAX - 1) {
        while (isspace((unsigned char)*token) && !in_quotes) token++;
        if (*token == '\0') break;
        cmd_buff->argv[cmd_buff->argc++] = token;
        while (*token) {
            if (*token == '"') {
                in_quotes = !in_quotes;
                memmove(token, token + 1, strlen(token));
                continue;
            }
            if (isspace((unsigned char)*token) && !in_quotes) {
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

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;
    char *line_copy = strdup(cmd_line);
    if (!line_copy) {
        return ERR_MEMORY;
    }
    char *saveptr;
    char *command = strtok_r(line_copy, "|", &saveptr);
    while (command != NULL && clist->num < CMD_MAX) {
        while (isspace((unsigned char)*command)) command++;
        cmd_buff_t *cmd = &clist->commands[clist->num];
        if (alloc_cmd_buff(cmd) != OK) {
            free(line_copy);
            return ERR_MEMORY;
        }
        if (build_cmd_buff(command, cmd) == OK) {
            clist->num++;
        }
        command = strtok_r(NULL, "|", &saveptr);
    }
    free(line_copy);
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }
    return OK;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        printf("exiting...\n");
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
   
    if (strcmp(cmd->argv[0], "rc") == 0) {
        printf("%d\n", last_return_code);
        last_return_code = 0;
        return BI_EXECUTED;
    }
    return BI_NOT_BI;
}

int execute_pipeline(command_list_t *clist) {
    int num_cmds = clist->num;
    int pipe_fds[2 * (num_cmds - 1)];
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipe_fds + i * 2) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        if (num_cmds == 1 && exec_built_in_cmd(&clist->commands[i]) == BI_EXECUTED) {
            return OK;
        }
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        }
        if (pids[i] == 0) { 
            if (i > 0) {
                dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < num_cmds - 1) {
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
            }
            for (int j = 0; j < 2 * (num_cmds - 1); j++) {
                close(pipe_fds[j]);
            }
            for (int j = 0; j < clist->commands[i].argc; j++) {
                if (!clist->commands[i].argv[j]) continue;
                if (strcmp(clist->commands[i].argv[j], "<") == 0) {
                    int fd = open(clist->commands[i].argv[j+1], O_RDONLY);
                    if (fd < 0) { perror("open"); exit(ERR_EXEC_CMD); }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    clist->commands[i].argv[j] = NULL;
                    clist->commands[i].argv[j+1] = NULL;
                } else if (strcmp(clist->commands[i].argv[j], ">") == 0) {
                    int fd = open(clist->commands[i].argv[j+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) { perror("open"); exit(ERR_EXEC_CMD); }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    clist->commands[i].argv[j] = NULL;
                    clist->commands[i].argv[j+1] = NULL;
                } else if (strcmp(clist->commands[i].argv[j], ">>") == 0) {
                    int fd = open(clist->commands[i].argv[j+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                    if (fd < 0) { perror("open"); exit(ERR_EXEC_CMD); }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    clist->commands[i].argv[j] = NULL;
                    clist->commands[i].argv[j+1] = NULL;
                }
            }
            int k = 0;
            for (int j = 0; j < clist->commands[i].argc; j++) {
                if (clist->commands[i].argv[j] != NULL) {
                    clist->commands[i].argv[k++] = clist->commands[i].argv[j];
                }
            }
            clist->commands[i].argv[k] = NULL;
           
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(ERR_EXEC_CMD);
        }
    }
 
    for (int i = 0; i < 2 * (num_cmds - 1); i++) {
        close(pipe_fds[i]);
    }
  
    for (int i = 0; i < num_cmds; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (i == num_cmds - 1) {  
            if (WIFEXITED(status)) {
                last_return_code = WEXITSTATUS(status);
            } else {
                last_return_code = 1;
            }
        }
    }
    return OK;
}

int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    command_list_t cmd_list;
    int rc;
    
    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_line, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_line[strcspn(cmd_line, "\n")] = '\0';
        
        if (strchr(cmd_line, '|') != NULL) {
            rc = build_cmd_list(cmd_line, &cmd_list);
            if (rc == WARN_NO_CMDS) {
                printf(CMD_WARN_NO_CMD);
                continue;
            }
            if (cmd_list.num == 1 && exec_built_in_cmd(&cmd_list.commands[0]) == BI_EXECUTED) {
                free_cmd_buff(&cmd_list.commands[0]);
                continue;
            }
            execute_pipeline(&cmd_list);
            for (int i = 0; i < cmd_list.num; i++) {
                free_cmd_buff(&cmd_list.commands[i]);
            }
        } else {
            cmd_buff_t cmd;
            if (alloc_cmd_buff(&cmd) != OK) {
                perror("alloc_cmd_buff");
                continue;
            }
            rc = build_cmd_buff(cmd_line, &cmd);
            if (rc == WARN_NO_CMDS) {
                printf(CMD_WARN_NO_CMD);
                free_cmd_buff(&cmd);
                continue;
            }
            if (exec_built_in_cmd(&cmd) == BI_EXECUTED) {
                free_cmd_buff(&cmd);
                continue;
            }
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                free_cmd_buff(&cmd);
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
            free_cmd_buff(&cmd);
        }
    }
    
    return OK;
}
